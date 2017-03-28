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

#include <iostream>
#include <algorithm>
#include <scif.h>
#include <cassert>
#include <cstring>
#include <thread>
#include <iostream>
#include <sstream>
#include <chrono>
#include <ctime>
#include <stdexcept>
#include <sys/types.h>
#include "trans4scif_config.h"
#include "rmawindow.h"
#include "hbsocket.h"

namespace t4s {

std::vector<uint8_t> HBSocket::notif{0xff};

void HBSocket::init() {
  RMAWindow buf_win = sn_.createRMAWindow(PAGE_SIZE, SCIF_PROT_READ | SCIF_PROT_WRITE);
  RMAWindow wr_win = sn_.createRMAWindow(WR_WIN_SIZE*PAGE_SIZE, SCIF_PROT_READ | SCIF_PROT_WRITE);
  std::fill_n(static_cast<Record *>(wr_win.get_mem()), wr_win.get_len()/sizeof(Record), inval_rec);

  // send the offsets to the peer
  std::vector<uint8_t> rmaids_msg;
  rmaids_msg.reserve(3*sizeof(RMAId));

  // Insert in message buf_id
  RMAId buf_id{buf_win.get_off(), buf_win.get_len()};
  std::vector<uint8_t> buf_msg(PackRMAIdMsg(buf_id));
  rmaids_msg.insert(rmaids_msg.end(), buf_msg.begin(), buf_msg.end());

  // Insert in message wr_id
  RMAId wr_id{wr_win.get_off(), wr_win.get_len()};
  std::vector<uint8_t> wr_msg(PackRMAIdMsg(wr_id));
  rmaids_msg.insert(rmaids_msg.end(), wr_msg.begin(), wr_msg.end());

  // Insert in message recvbuf_id
  RMAId recvbuf_id{recvbuf_.get_off(), recvbuf_.get_len()};
  std::vector<uint8_t> recvbuf_msg(PackRMAIdMsg(recvbuf_id));
  rmaids_msg.insert(rmaids_msg.end(), recvbuf_msg.begin(), recvbuf_msg.end());
  // Send the message to the peer
  sn_.sendMsg(rmaids_msg);

  // Receive the peer node's RMAIds
  rmaids_msg = sn_.recvMsg(3*sizeof(RMAId));
  RMAId peer_buf_id(UnpackRMAIdMsg(rmaids_msg));
  RMAId peer_wr_id(UnpackRMAIdMsg(rmaids_msg));
  peer_recvbuf_ = UnpackRMAIdMsg(rmaids_msg);
  assert(rmaids_msg.empty());
  assert(peer_recvbuf_.size == sendbuf_.get_len());

  // Create the RMARecords
  Mmapmem peer_buf_mmap(sn_.createMmapmem(peer_buf_id.off,
                                          peer_buf_id.size, SCIF_PROT_READ | SCIF_PROT_WRITE));
  Mmapmem peer_wr_mmap(sn_.createMmapmem(peer_wr_id.off,
                                          peer_wr_id.size, SCIF_PROT_READ | SCIF_PROT_WRITE));

  sendrecs_.reset(new RMARecordsWriter(buf_win, peer_wr_mmap, recv_buf_size_));
  recvrecs_.reset(new RMARecordsReader(peer_buf_mmap, wr_win, recv_buf_size_));
}

HBSocket::HBSocket(uint16_t target_node_id, uint16_t target_port, std::size_t buf_size) :
    sn_(target_node_id, target_port),
    recvbuf_(sn_.createRMAWindow(buf_size, SCIF_PROT_WRITE)),
    sendbuf_(sn_.createRMAWindow(buf_size, SCIF_PROT_READ)),
    recv_buf_size_(buf_size) {
  init();
}

HBSocket::HBSocket(uint16_t listening_port, std::size_t buf_size) :
    sn_(listening_port),
    recvbuf_(sn_.createRMAWindow(buf_size, SCIF_PROT_WRITE)),
    sendbuf_(sn_.createRMAWindow(buf_size, SCIF_PROT_READ)),
    recv_buf_size_(buf_size) {
  init();
}

HBSocket::HBSocket(ScifEpd &epd, std::size_t buf_size) :
    sn_(epd),
    recvbuf_(sn_.createRMAWindow(buf_size, SCIF_PROT_WRITE)),
    sendbuf_(sn_.createRMAWindow(buf_size, SCIF_PROT_READ)),
    recv_buf_size_(buf_size) {
  init();
}

Buffer HBSocket::getSendBuffer() {
  auto buf_rec = sendrecs_->getBufRec();
  //round towards minus infinity
  std::size_t space = ROUND_TO_BOUNDARY((buf_rec.end-buf_rec.start)-(CACHELINE_SIZE-1),
                                        CACHELINE_SIZE);
  space = std::min(space, recv_buf_size_/2);
  return Buffer{sendbuf_.get_mem()+buf_rec.start, space};
}

std::size_t HBSocket::send(const uint8_t *data, std::size_t data_size) {
  if (!data_size || !sendrecs_->canWrite() )
    return 0;
  auto buf_rec = sendrecs_->getBufRec();
  //round towards minus infinity
  std::size_t space = ROUND_TO_BOUNDARY((buf_rec.end-buf_rec.start)-(CACHELINE_SIZE-1),
                                        CACHELINE_SIZE);
  std::size_t to_write = std::min(std::min(space, data_size), recv_buf_size_/2);
  assert(to_write > 0);
  if (!sendbuf_.in_window(static_cast<const uint8_t *>(data))) {
    void *__restrict dest = static_cast<void *__restrict>(sendbuf_.get_mem()) + buf_rec.start;
    memcpy(dest, data, to_write);
  } else {
    assert(to_write == data_size);
  }
  sn_.writeMsg(peer_recvbuf_.off+buf_rec.start, sendbuf_.get_off()+buf_rec.start,
               ROUND_TO_BOUNDARY(to_write, CACHELINE_SIZE));
  if (1 != sn_.send(notif.data(), 1))
    sn_.sendMsg(notif);
  uint64_t sigval = to_write + buf_rec.start;
  off_t sigoff = sendrecs_->written(to_write);
  sn_.signalPeer(sigoff, sigval);

  return to_write + send(data+to_write, data_size-to_write);
}

std::size_t HBSocket::recv(uint8_t *data, std::size_t data_size) {
  // Receive as many pending notification tokens as possible
  if (pending_notifs > 0) {
    uint8_t tmp[pending_notifs];
    pending_notifs -= sn_.recv(tmp, pending_notifs);
  }

  if (!data_size || !recvrecs_->canRead())
    return 0;

  auto wr_rec = recvrecs_->getWrRec();
  assert (wr_rec.end > wr_rec.start);
  std::size_t to_read = std::min(wr_rec.end-wr_rec.start, data_size);
  void * __restrict dest = data;
  void * __restrict src = static_cast<void * __restrict>(recvbuf_.get_mem())+wr_rec.start;
  memcpy(data, src, to_read);

  if (recvrecs_->read(to_read))
    pending_notifs++;

  return to_read + recv(data+to_read, data_size-to_read);
}

void HBSocket::waitIn (long timeout) {
    if (pending_notifs > 0) {
      sn_.recvMsg(pending_notifs);
      pending_notifs = 0;
    }
    while (!sn_.canRecvMsg(timeout));
}

}
