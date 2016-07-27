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
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <sys/types.h>
#include "trans4scif_config.h"
#include "rmawindow.h"
#include "ctl_messages.h"
#include "hbsocket.h"

namespace t4s {


void HBSocket::Init() {
  // Send to peer node the recvbuf_ details
  RMAId my_id{recvbuf_.get_wr_rmaoff(), recvbuf_.get_space()};
  std::vector<uint8_t> msg_send(PackRMAIdMsg(my_id));
  sn_.SendMsg(msg_send);

  // Get the peer node's recvbuf_ details
  std::vector<uint8_t> msg_recv = sn_.RecvMsg(sizeof(RMAId));
  RMAId peer_id(UnpackRMAIdMsg(msg_recv));

  // Init the rem_recvbuf_ and sendbuf_
  rem_recvbuf_.reset(new VirtCircbuf(peer_id.off, peer_id.size));
  sendbuf_.reset(new Circbuf(sn_.CreateRMAWindow(peer_id.size, SCIF_PROT_READ)));
}

HBSocket::HBSocket(uint16_t target_node_id, uint16_t target_port) :
    sn_(target_node_id, target_port),
    recvbuf_(sn_.CreateRMAWindow(RECV_BUF_SIZE, SCIF_PROT_WRITE)) {
  Init();
}

HBSocket::HBSocket(uint16_t listening_port) :
    sn_(listening_port),
    recvbuf_(sn_.CreateRMAWindow(RECV_BUF_SIZE, SCIF_PROT_WRITE)) {
  Init();
}

HBSocket::HBSocket(ScifEpd &epd) :
    sn_(epd),
    recvbuf_(sn_.CreateRMAWindow(RECV_BUF_SIZE, SCIF_PROT_WRITE))  {
  Init();
}

std::size_t HBSocket::Send(const uint8_t *msg_it, std::size_t msg_size) {
  if (msg_size == 0) {
    return 0;
  }
  GetRemRecvbufNotifs();
  if (!rem_recvbuf_->get_space()) {
    return 0;
  }
  off_t sync_off = rem_recvbuf_->get_wr_rmaoff();
  off_t dest_off = rem_recvbuf_->get_wr_rmaoff();
  off_t src_off = sendbuf_->get_wr_rmaoff();

  //  Reset the chunk head field to 0. It should be writen by scif_signal
  sendbuf_->WrResetChunkHead();
  rem_recvbuf_->WrAdvance(CHUNK_HEAD_SIZE);

  //  Write the data
  auto write_data = [this, &msg_it, &msg_size]() -> std::size_t {
    std::size_t sendbuf_written = sendbuf_->Write(msg_it, msg_size);
    std::size_t rem_recvbuf_advanced = rem_recvbuf_->WrAdvance(msg_size);
    assert(sendbuf_written == rem_recvbuf_advanced);
    sendbuf_->WrAlign();
    rem_recvbuf_->WrAlign();
    msg_it += sendbuf_written;
    msg_size -= sendbuf_written;
    return sendbuf_written;
  };
  std::size_t written = write_data();

  //  Issue the RDMA write
  sn_.WriteMsg(dest_off, src_off, ROUND_TO_BOUNDARY(CHUNK_HEAD_SIZE + written, CACHELINE_SIZE));
  std::size_t total_size_send = written;


  //  Check if we can write some more data (circbuf wrap around case)
  if (rem_recvbuf_->get_space() && msg_size) {
  //  Get the offsets to initiate the RMDA write and synchronization
    off_t src_off = sendbuf_->get_wr_rmaoff();
    off_t dest_off = rem_recvbuf_->get_wr_rmaoff();
    std::size_t written = write_data();
  //  Issue the RDMA write (cache aligne the end as well)
    sn_.WriteMsg(dest_off, src_off, ROUND_TO_BOUNDARY(written, CACHELINE_SIZE));
    total_size_send += written;
  }

  //  write (remote) chunk head
  sn_.SignalPeer(sync_off, total_size_send);

  return total_size_send;
}

//  TODO: Consider implementing the blocking alternatives as well
std::size_t HBSocket::Recv(uint8_t *data, std::size_t data_size) {
  if (data_size == 0) {
    return 0;
  }
  UpdateRecvbufSpace();
  if (recvbuf_.is_empty()) {
    return 0;
  }
  // Currently the chunk header contains only the size of the chunk
  std::size_t chunk_size = recvbuf_.RdReadResetChunkHead();

  std::size_t bytes = recvbuf_.Read(data, std::min(chunk_size, data_size));
  chunk_size -= bytes;
  data_size -= bytes;

  //  Truncate
  std::size_t trunc_data_size = 0;
  if (chunk_size) {
    assert(!recvbuf_.is_empty());
    assert(data_size == 0);
    std::vector<uint8_t> trunc(chunk_size);
    trunc_data_size = recvbuf_.Read(trunc.data(), chunk_size);
    assert(chunk_size == trunc_data_size);
    std::cerr << "Warning: Recv truncated: " << "[" << trunc_data_size << "B]" << std::endl;
  }
  recvbuf_.RdAlign();

  //  Notify sender
  std::vector<uint8_t> recv_notif_msg;
  inttype_to_vec_le(bytes + trunc_data_size, recv_notif_msg);
  sn_.SendMsg(recv_notif_msg);

  return bytes;
}

std::vector<uint8_t> HBSocket::Recv() {
  UpdateRecvbufSpace();
  if (recvbuf_.is_empty()) {
    std::vector<uint8_t> empty_v;
    return empty_v;
  }

  //  Currently the chunk header contains only the size of the chunk
  std::size_t chunk_size = recvbuf_.RdReadResetChunkHead();
  std::vector<uint8_t> msg(chunk_size);
  chunk_size -= recvbuf_.Read(msg.data(), chunk_size);
  assert(!chunk_size);
  recvbuf_.RdAlign();

  //  Notify sender
  std::vector<uint8_t> recv_notif_msg;
  inttype_to_vec_le(msg.size(), recv_notif_msg);
  sn_.SendMsg(recv_notif_msg);

  // Assuming copy elision
  return msg;
}

void HBSocket::UpdateRecvbufSpace() {
  std::size_t chunk_size = 0;
  while (recvbuf_.get_space() && (chunk_size = recvbuf_.WrReadChunkHead()))
    recvbuf_.WrFullAdvanceAlign(CHUNK_HEAD_SIZE + chunk_size);
}

void HBSocket::GetRemRecvbufNotifs() {
  while (sn_.HasRecvMsg()) {
    std::vector<uint8_t> recv_notif_msg = sn_.RecvMsg(sizeof(std::size_t));
    std::size_t msg_size;
    vec_to_inttype_le(recv_notif_msg, msg_size);
    rem_recvbuf_->RdFullAdvanceAlign(CHUNK_HEAD_SIZE + msg_size);
    sendbuf_->RdFullAdvanceAlign(CHUNK_HEAD_SIZE + msg_size);
    assert(rem_recvbuf_->get_space() == sendbuf_->get_space());
  }
}

}