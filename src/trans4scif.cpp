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
  ScifNode sn_;
  RMAWindow recvbuf_;
  RMAWindow sendbuf_;
  RMAId peer_recvbuf_;
  const std::size_t recv_buf_size_;
  std::unique_ptr<RMARecordsWriter> sendrecs_;
  std::unique_ptr<RMARecordsReader> recvrecs_;
  int pending_notifs = 0;

  explicit SockState(uint16_t target_node_id, uint16_t target_port, std::size_t buf_size) :
      sn_(target_node_id, target_port),
      recvbuf_(sn_.createRMAWindow(buf_size, SCIF_PROT_WRITE)),
      sendbuf_(sn_.createRMAWindow(buf_size, SCIF_PROT_READ)),
      recv_buf_size_(buf_size) {
    init();
  }

  explicit SockState(uint16_t listening_port, std::size_t buf_size) :
      sn_(listening_port),
      recvbuf_(sn_.createRMAWindow(buf_size, SCIF_PROT_WRITE)),
      sendbuf_(sn_.createRMAWindow(buf_size, SCIF_PROT_READ)),
      recv_buf_size_(buf_size) {
    init();
  }
//
//  explicit Socket::SockState(ScifEpd &epd, std::size_t buf_size) :
//      sn_(epd),
//      recvbuf_(sn_.createRMAWindow(buf_size, SCIF_PROT_WRITE)),
//      sendbuf_(sn_.createRMAWindow(buf_size, SCIF_PROT_READ)),
//      recv_buf_size_(buf_size) {
//    init();
//  }
};

void Socket::SockState::init() {
  auto buf_win = sn_.createRMAWindow(PAGE_SIZE, SCIF_PROT_READ | SCIF_PROT_WRITE);
  auto wr_win = sn_.createRMAWindow(WR_WIN_SIZE*PAGE_SIZE, SCIF_PROT_READ | SCIF_PROT_WRITE);
  std::fill_n(static_cast<Record *>(wr_win.get_mem()), wr_win.get_len()/sizeof(Record), inval_rec);

  // send the offsets to the peer
  std::vector<uint8_t> rmaids_msg;
  rmaids_msg.reserve(3*sizeof(RMAId));

  // Insert in message buf_id
  RMAId buf_id{buf_win.get_off(), buf_win.get_len()};
  auto buf_msg(PackRMAIdMsg(buf_id));
  rmaids_msg.insert(rmaids_msg.end(), buf_msg.begin(), buf_msg.end());

  // Insert in message wr_id
  RMAId wr_id{wr_win.get_off(), wr_win.get_len()};
  auto wr_msg(PackRMAIdMsg(wr_id));
  rmaids_msg.insert(rmaids_msg.end(), wr_msg.begin(), wr_msg.end());

  // Insert in message recvbuf_id
  RMAId recvbuf_id{recvbuf_.get_off(), recvbuf_.get_len()};
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
  assert(peer_recvbuf_.size == sendbuf_.get_len());

  // Create the RMARecords
  auto peer_buf_mmap(sn_.createMmapmem(peer_buf_id.off,
                                          peer_buf_id.size, SCIF_PROT_READ | SCIF_PROT_WRITE));
  auto peer_wr_mmap(sn_.createMmapmem(peer_wr_id.off,
                                         peer_wr_id.size, SCIF_PROT_READ | SCIF_PROT_WRITE));

  sendrecs_.reset(new RMARecordsWriter(buf_win, peer_wr_mmap, recv_buf_size_));
  recvrecs_.reset(new RMARecordsReader(peer_buf_mmap, wr_win, recv_buf_size_));
}

Socket::Socket(uint16_t target_node_id, uint16_t target_port, std::size_t buf_size) :
    state(new SockState(target_node_id, target_port, buf_size)) {}

Socket::Socket(uint16_t listening_port, std::size_t buf_size) :
    state(new SockState(listening_port, buf_size)) {}

//
//Socket::Socket(ScifEpd &epd, std::size_t buf_size) :
//    sn_(epd),
//    recvbuf_(sn_.createRMAWindow(buf_size, SCIF_PROT_WRITE)),
//    sendbuf_(sn_.createRMAWindow(buf_size, SCIF_PROT_READ)),
//    recv_buf_size_(buf_size) {
//  init();
//}

Socket::~Socket() = default;


Blk Socket::getSendBuffer() {
  auto buf_rec = state->sendrecs_->getBufRec();
  //round towards minus infinity
  std::size_t space = ROUND_TO_BOUNDARY((buf_rec.end-buf_rec.start)-(CACHELINE_SIZE-1),
                                        CACHELINE_SIZE);
  space = std::min(space, state->recv_buf_size_/2);
  return Blk{state->sendbuf_.get_mem()+buf_rec.start, space};
}

std::size_t Socket::send(const uint8_t *data, std::size_t data_size) {
  std::size_t total_bytes = 0;
  for (int i = 0; i < 2; ++i) {
    if (!data_size || !state->sendrecs_->canWrite())
      return total_bytes;
    auto buf_rec = state->sendrecs_->getBufRec();
    //round towards minus infinity
    std::size_t space = ROUND_TO_BOUNDARY((buf_rec.end - buf_rec.start) - (CACHELINE_SIZE - 1),
                                          CACHELINE_SIZE);
    std::size_t to_write = std::min(std::min(space, data_size), state->recv_buf_size_ / 2);
    assert(to_write > 0);

    if (!state->sendbuf_.in_window(static_cast<const uint8_t *>(data))) {
      void *__restrict dest = static_cast<void *__restrict>(state->sendbuf_.get_mem()) + buf_rec.start;
      memcpy(dest, data, to_write);
    } else {
      assert(to_write == data_size);
    }
    state->sn_.writeMsg(state->peer_recvbuf_.off + buf_rec.start, state->sendbuf_.get_off() + buf_rec.start,
                 ROUND_TO_BOUNDARY(to_write, CACHELINE_SIZE));
    if (1 != state->sn_.send(notif.data(), 1))
      state->sn_.sendMsg(notif);
    uint64_t sigval = to_write + buf_rec.start;
    off_t sigoff = state->sendrecs_->written(to_write);
    state->sn_.signalPeer(sigoff, sigval);

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
    if (state->pending_notifs > 0) {
      uint8_t tmp[state->pending_notifs];
      state->pending_notifs -= state->sn_.recv(tmp, state->pending_notifs);
    }

    if (!data_size || !state->recvrecs_->canRead())
      return total_bytes;

    auto wr_rec = state->recvrecs_->getWrRec();
    assert(wr_rec.end > wr_rec.start);
    std::size_t to_read = std::min(wr_rec.end - wr_rec.start, data_size);
    void *__restrict dest = data;
    void *__restrict src = static_cast<void *__restrict>(state->recvbuf_.get_mem()) + wr_rec.start;
    memcpy(data, src, to_read);

    if (state->recvrecs_->read(to_read))
      state->pending_notifs++;

    data += to_read;
    data_size -= to_read;
    total_bytes += to_read;
  }
  return total_bytes;
}

void Socket::waitIn (long timeout) {
  if (state->pending_notifs > 0) {
    state->sn_.recvMsg(state->pending_notifs);
    state->pending_notifs = 0;
  }
  while (!state->sn_.canRecvMsg(timeout));
}

//Socket* epdSocket(scif_epd_t epd, std::size_t buf_size) {
//  assert(epd != SCIF_OPEN_FAILED);
//  ScifEpd e(epd);
//  return new Socket(e, buf_size);
//}
//
//std::future<Socket *> epdSocketAsync(scif_epd_t epd, std::size_t buf_size) {
//  assert(epd != SCIF_OPEN_FAILED);
//  return std::async(std::launch::async, [epd, buf_size]() -> Socket * {
//    ScifEpd e(epd);
//    return new Socket(e, buf_size);
//  });
//}

}
