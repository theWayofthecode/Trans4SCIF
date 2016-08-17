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

#include <system_error>
#include <stdexcept>
#include <cassert>
#include "scifnode.h"
#include "trans4scif_config.h"

namespace t4s {

ScifNode::ScifNode(uint16_t target_node_id, uint16_t target_port) {
//   connect
  struct scif_portID target_addr;
  target_addr.node = target_node_id;
  target_addr.port = target_port;
  if (scif_connect(epd_.get(), &target_addr) == -1)
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);
}

ScifNode::ScifNode(uint16_t listening_port) {
  // bind
  if (scif_bind(epd_.get(), listening_port) == -1)
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);

  // listen (backlog = 1)
  if (scif_listen(epd_.get(), 1) == -1)
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);

  // accept
  scif_epd_t acc_epd_t;
  struct scif_portID peer_addr;
  if (scif_accept(epd_.get(), &peer_addr, &acc_epd_t, SCIF_ACCEPT_SYNC) == -1)
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);
  epd_ = ScifEpd(acc_epd_t);
}

std::size_t ScifNode::transmission(int(*trans_prim)(scif_epd_t, void *, int, int),
                                   std::vector<uint8_t>::iterator start,
                                   std::vector<uint8_t>::iterator end) {
  auto it = start;
  int i;
  for (i = 0; i < SCIF_TRANS_RETRIES; ++i) {
    int bytes = trans_prim(epd_.get(), &(*it), std::distance(it, end), 0);
    if (bytes == -1)
      throw std::system_error(errno, std::system_category(), __FILE__LINE__);
    std::advance(it, bytes);
    if (std::distance(it, end) == 0)
      break;
    scaled_sleep(i, SCIF_TRANS_RETRIES/2, SCIF_TRANS_RETRIES - 100);
  }
  if (i == SCIF_TRANS_RETRIES)
    throw std::runtime_error("scif_send/recv retries exhausted.");
  return std::distance(start, it);
}

std::size_t ScifNode::sendMsg(std::vector<uint8_t> &payload) {
  return transmission(scif_send, payload.begin(), payload.end());
}

std::vector<uint8_t> ScifNode::recvMsg(std::size_t size) {
  std::vector<uint8_t> payload(size);
  auto bytes_recv = transmission(scif_recv, payload.begin(), payload.end());
  assert(size == bytes_recv);
  return payload;
}

void ScifNode::writeMsg(off_t dest, off_t src, std::size_t len) {
  if (scif_writeto(epd_.get(), src, len, dest, 0) == -1)
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);
}

void ScifNode::signalPeer(off_t dest, std::uint64_t val) {
  if (scif_fence_signal(epd_.get(), 0, 0, dest, val, SCIF_FENCE_INIT_SELF | SCIF_SIGNAL_REMOTE) == -1)
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);
}

bool ScifNode::hasRecvMsg() {
  struct scif_pollepd pepd;
  pepd.epd = epd_.get();
  pepd.events = SCIF_POLLIN;
  pepd.revents = 0;
  if (scif_poll(&pepd, 1, 0) == -1)
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);
  return pepd.revents == SCIF_POLLIN;
}

}
