/*
	Copyright (c) 2015-2016 CERN
	
	This software is distributed under the terms of the 
	GNU Lesser General Public Licence version 3 (LGPLv3),
	copied verbatim in the file "LICENSE".
	In applying this licence, CERN does not waive 
	the privileges and immunities granted to it by virtue of its status 
	as an Intergovernmental Organization or submit itself to any jurisdiction.
	
	Author: Aram Santogidis <aram.santogidis@cern.ch>
*/
#include "trans4scif.h"

#include <iostream>
#include <algorithm>
#include <scif.h>
#include <cassert>
#include <sstream>
#include <cstring>
#include <array>
#include <sys/types.h>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#include <tbb/task_scheduler_init.h>
#include "scifnode.h"
#include "ctl_messages.h"
#include "rmarecordsreader.h"
#include "rmarecordswriter.h"
#include "trans4scif_config.h"

namespace t4s {

static std::vector<uint8_t> notif{0xff};

std::string trans4scif_config() {
  std::stringstream ss;
  ss << "Version=" << TRANS4SCIF_VERSION_MAJOR
     << "." << TRANS4SCIF_VERSION_MINOR << std::endl;
  ss << "default BUF_SIZE=" << BUF_SIZE << "B\n";
  return ss.str();
}

class Socket::SockState {
 private:
  void init();

 public:
  ScifNode const sn_;
  RMAWindow const recvbuf_;
  RMAWindow const sendbuf_;
  RMAId peer_recvbuf_;
  std::size_t const recv_buf_size_;
  std::unique_ptr<RMARecordsWriter> sendrecs_;
  std::unique_ptr<RMARecordsReader> recvrecs_;
  int pending_notifs_ = 0;
  tbb::task_scheduler_init tbbts_;


  explicit SockState(uint16_t target_node_id, uint16_t target_port, std::size_t buf_size) :
      sn_(target_node_id, target_port),
      recvbuf_(sn_.createRMAWindow(buf_size, SCIF_PROT_WRITE)),
      sendbuf_(sn_.createRMAWindow(buf_size, SCIF_PROT_READ)),
      recv_buf_size_(buf_size) { init(); }

  explicit SockState(uint16_t listening_port, std::size_t buf_size) :
      sn_(listening_port),
      recvbuf_(sn_.createRMAWindow(buf_size, SCIF_PROT_WRITE)),
      sendbuf_(sn_.createRMAWindow(buf_size, SCIF_PROT_READ)),
      recv_buf_size_(buf_size) { init(); }

  explicit SockState(ScifEpd& epd, std::size_t buf_size) :
      sn_(epd),
      recvbuf_(sn_.createRMAWindow(buf_size, SCIF_PROT_WRITE)),
      sendbuf_(sn_.createRMAWindow(buf_size, SCIF_PROT_READ)),
      recv_buf_size_(buf_size) { init(); }
};

void Socket::SockState::init() {
  auto buf_win = sn_.createRMAWindow(PAGE_SIZE, SCIF_PROT_READ | SCIF_PROT_WRITE);
  auto wr_win = sn_.createRMAWindow(WR_WIN_SIZE*PAGE_SIZE, SCIF_PROT_READ | SCIF_PROT_WRITE);
  std::fill_n(static_cast<Record *>(wr_win.mem()), wr_win.size()/sizeof(Record), inval_rec);

  // send the offsets to the peer
  std::vector<uint8_t> rmaids_msg;
  rmaids_msg.reserve(3*sizeof(RMAId));

  // Insert in message buf_id
  RMAId buf_id{buf_win.off(), buf_win.size()};
  auto buf_msg(PackRMAIdMsg(buf_id));
  rmaids_msg.insert(rmaids_msg.end(), buf_msg.begin(), buf_msg.end());

  // Insert in message wr_id
  RMAId wr_id{wr_win.off(), wr_win.size()};
  auto wr_msg(PackRMAIdMsg(wr_id));
  rmaids_msg.insert(rmaids_msg.end(), wr_msg.begin(), wr_msg.end());

  // Insert in message recvbuf_id
  RMAId recvbuf_id{recvbuf_.off(), recvbuf_.size()};
  auto recvbuf_msg(PackRMAIdMsg(recvbuf_id));
  rmaids_msg.insert(rmaids_msg.end(), recvbuf_msg.begin(), recvbuf_msg.end());
  // Send the message to the peer
  sn_.sendMsg(rmaids_msg);

  // Receive the peer node's RMAIds
  rmaids_msg = sn_.recvMsg(3*sizeof(RMAId));
  auto peer_buf_id(UnpackRMAIdMsg(rmaids_msg));
  auto peer_wr_id(UnpackRMAIdMsg(rmaids_msg));
  peer_recvbuf_ = UnpackRMAIdMsg(rmaids_msg);
  assert(rmaids_msg.empty());
  assert(peer_recvbuf_.size == sendbuf_.size());

  // Create the RMARecords
  auto peer_buf_mmap(sn_.createMmapmem(peer_buf_id.off,
                                          peer_buf_id.size, SCIF_PROT_READ | SCIF_PROT_WRITE));
  auto peer_wr_mmap(sn_.createMmapmem(peer_wr_id.off,
                                         peer_wr_id.size, SCIF_PROT_READ | SCIF_PROT_WRITE));

  sendrecs_.reset(new RMARecordsWriter(buf_win, peer_wr_mmap, recv_buf_size_));
  recvrecs_.reset(new RMARecordsReader(peer_buf_mmap, wr_win, recv_buf_size_));
}

Socket::Socket(uint16_t target_node_id, uint16_t target_port, std::size_t buf_size) :
    s(new SockState(target_node_id, target_port, buf_size)) {}

Socket::Socket(uint16_t listening_port, std::size_t buf_size) :
    s(new SockState(listening_port, buf_size)) {}

Socket::Socket(ScifEpd& epd, std::size_t buf_size) :
    s(new SockState(epd, buf_size)) {}

Socket::~Socket() = default;

//TODO: const this method (and check in other modules as well)
Blk Socket::getSendBuffer() const {
  auto buf_rec = s->sendrecs_->getBufRec();
  //round towards minus infinity
  std::size_t space = round<CL_SIZE>((buf_rec.end-buf_rec.start)-(CL_SIZE-1));
  space = std::min(space, s->recv_buf_size_/2);
  return Blk{s->sendbuf_.mem()+buf_rec.start, space};
}

std::size_t Socket::send(uint8_t *data, std::size_t data_size) {
  std::size_t total_bytes = 0;
  for (int i = 0; i < 2; ++i) {
    if (!data_size || !s->sendrecs_->canWrite())
      return total_bytes;
    auto buf_rec = s->sendrecs_->getBufRec();
    //round towards minus infinity
    std::size_t space = round<CL_SIZE>((buf_rec.end-buf_rec.start)-(CL_SIZE-1));
    std::size_t to_write = std::min(std::min(space, data_size), s->recv_buf_size_ / 2);
    assert(to_write > 0);

    if (!s->sendbuf_.isInWindow(data)) {
      //TODO: static cast src as well. Double check if this make sense at all
      auto dest = static_cast<uint8_t *__restrict>(s->sendbuf_.mem() + buf_rec.start);
      auto src = static_cast<uint8_t *__restrict>(data);
      if (to_write < MEMCPY_SINGLE_CHUNK_THRESH)
        memcpy(dest, src, to_write);
      else
        tbb::parallel_for(tbb::blocked_range<size_t>(0,to_write, to_write/MEMCPY_CHUNKS),
                          [dest, src](const tbb::blocked_range<size_t>& r) {
          std::memcpy(dest + r.begin(), src + r.begin(), r.end()-r.begin());
        });
    } else {
      assert(to_write == data_size);
    }
    s->sn_.writeMsg(s->peer_recvbuf_.off + buf_rec.start,
                        s->sendbuf_.off() + buf_rec.start,
                        round<CL_SIZE>(to_write));
    if (1 != s->sn_.send(notif.data(), 1))
      s->sn_.sendMsg(notif);
    uint64_t sigval = to_write + buf_rec.start;
    off_t sigoff = s->sendrecs_->written(to_write);
    s->sn_.signalPeer(sigoff, sigval);

    data += to_write;
    data_size -= to_write;
    total_bytes += to_write;
  }
  return total_bytes;
}

std::size_t Socket::recv(uint8_t *data, std::size_t data_size) {
  std::size_t total_bytes = 0;
  for (int i = 0; i < 2; ++i) {
    // Receive as many pending notification tokens as possible
    if (s->pending_notifs_ > 0) {
      uint8_t tmp[s->pending_notifs_];
      s->pending_notifs_ -= s->sn_.recv(tmp, s->pending_notifs_);
    }

    if (!data_size || !s->recvrecs_->canRead())
      return total_bytes;

    auto wr_rec = s->recvrecs_->getWrRec();
    assert(wr_rec.end > wr_rec.start);
    std::size_t to_read = std::min(wr_rec.end - wr_rec.start, data_size);
    uint8_t *__restrict dest = data;
    auto src = static_cast<uint8_t *__restrict>(s->recvbuf_.mem()) + wr_rec.start;
    if (to_read < MEMCPY_SINGLE_CHUNK_THRESH)
      memcpy(dest, src, to_read);
    else
      tbb::parallel_for(tbb::blocked_range<size_t>(0,to_read, to_read/MEMCPY_CHUNKS),
                        [dest, src](const tbb::blocked_range<size_t>& r) {
                          std::memcpy(dest + r.begin(), src + r.begin(), r.end()-r.begin());
                        });
    if (s->recvrecs_->read(to_read))
      s->pending_notifs_++;

    data += to_read;
    data_size -= to_read;
    total_bytes += to_read;
  }
  return total_bytes;
}

void Socket::waitIn (long timeout) {
  if (s->pending_notifs_ > 0) {
    s->sn_.recvMsg(s->pending_notifs_);
    s->pending_notifs_ = 0;
  }
  while (!s->sn_.canRecvMsg(timeout));
}

}
