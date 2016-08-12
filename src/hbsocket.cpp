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

using hrclock = std::chrono::high_resolution_clock;

template<typename DurationType>
inline std::chrono::microseconds cast_microseconds(DurationType d) {
  return std::chrono::duration_cast<std::chrono::microseconds>(d);
}

void HBSocket::Init() {
  RMAWindow buf_win = sn_.CreateRMAWindow(PAGE_SIZE, SCIF_PROT_READ | SCIF_PROT_WRITE);
  RMAWindow wr_win = sn_.CreateRMAWindow(WR_WIN_SIZE*PAGE_SIZE, SCIF_PROT_READ | SCIF_PROT_WRITE);
  std::fill_n(static_cast<Record *>(wr_win.get_mem()), wr_win.get_len()/sizeof(Record), inval_rec);

  // Send the offsets to the peer
  std::vector<uint8_t> rmaids_msg;
  rmaids_msg.reserve(3*sizeof(RMAId));

  RMAId buf_id{buf_win.get_off(), buf_win.get_len()};
  std::vector<uint8_t> buf_msg(PackRMAIdMsg(buf_id));
  rmaids_msg.insert(rmaids_msg.end(), buf_msg.begin(), buf_msg.end());

  RMAId wr_id{wr_win.get_off(), wr_win.get_len()};
  std::vector<uint8_t> wr_msg(PackRMAIdMsg(wr_id));
  rmaids_msg.insert(rmaids_msg.end(), wr_msg.begin(), wr_msg.end());

  RMAId recvbuf_id{recvbuf_.get_off(), recvbuf_.get_len()};
  std::vector<uint8_t> recvbuf_msg(PackRMAIdMsg(recvbuf_id));
  rmaids_msg.insert(rmaids_msg.end(), recvbuf_msg.begin(), recvbuf_msg.end());
  sn_.SendMsg(rmaids_msg);

  // Get the peer node's offsets
  rmaids_msg = sn_.RecvMsg(3*sizeof(RMAId));
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

  sendrecs_.reset(new RMARecordsWriter(buf_win, peer_wr_mmap));
  recvrecs_.reset(new RMARecordsReader(peer_buf_mmap, wr_win));
}

HBSocket::HBSocket(uint16_t target_node_id, uint16_t target_port) :
    sn_(target_node_id, target_port),
    recvbuf_(sn_.CreateRMAWindow(RECV_BUF_SIZE, SCIF_PROT_WRITE)),
    sendbuf_(sn_.CreateRMAWindow(SEND_BUF_SIZE, SCIF_PROT_READ)) {
  Init();
}

HBSocket::HBSocket(uint16_t listening_port) :
    sn_(listening_port),
    recvbuf_(sn_.CreateRMAWindow(RECV_BUF_SIZE, SCIF_PROT_WRITE)),
    sendbuf_(sn_.CreateRMAWindow(SEND_BUF_SIZE, SCIF_PROT_READ)) {
  Init();
}

HBSocket::HBSocket(ScifEpd &epd) :
    sn_(epd),
    recvbuf_(sn_.CreateRMAWindow(RECV_BUF_SIZE, SCIF_PROT_WRITE)),
    sendbuf_(sn_.CreateRMAWindow(SEND_BUF_SIZE, SCIF_PROT_READ)) {
  Init();
}

std::size_t HBSocket::Send(const uint8_t *msg_it, std::size_t msg_size) {
  if (!sendrecs_->canWrite() || !msg_size)
    return 0;
  auto buf_rec = sendrecs_->getBufRec();
  //round towards minus infinity
  std::size_t space = ROUND_TO_BOUNDARY((buf_rec.end-buf_rec.start)-(CACHELINE_SIZE-1),
                                        CACHELINE_SIZE);
  std::size_t to_write = std::min(space, msg_size);
  assert(to_write > 0);
  void * __restrict dest = static_cast<void * __restrict>(sendbuf_.get_mem())+buf_rec.start;
  memcpy(dest, msg_it, to_write);
  sn_.WriteMsg(peer_recvbuf_.off+buf_rec.start, sendbuf_.get_off()+buf_rec.start,
               ROUND_TO_BOUNDARY(to_write, CACHELINE_SIZE));
  uint64_t sigval = to_write + buf_rec.start;
  off_t sigoff = sendrecs_->written(to_write);
  sn_.SignalPeer(sigoff, sigval);
  return to_write + Send(msg_it+to_write, msg_size-to_write);
}

//  TODO: Consider implementing the blocking alternatives as well
std::size_t HBSocket::Recv(uint8_t *data, std::size_t data_size) {
  if (!recvrecs_->canRead() || !data_size)
    return 0;
  auto wr_rec = recvrecs_->getWrRec();
  assert (wr_rec.end > wr_rec.start);
  std::size_t to_read = std::min(wr_rec.end-wr_rec.start, data_size);
  void * __restrict dest = data;
  void * __restrict src = static_cast<void * __restrict>(recvbuf_.get_mem())+wr_rec.start;
  memcpy(data, src, to_read);
  recvrecs_->read(to_read);
  return to_read + Recv(data+to_read, data_size-to_read);
}

}
