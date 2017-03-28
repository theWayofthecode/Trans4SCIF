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

#ifndef _SOCKET_SOCKET_H_
#define _SOCKET_SOCKET_H_

#include <vector>
#include <cstdint>
#include <iostream>
#include "scifnode.h"
#include "ctl_messages.h"
#include "rmarecordsreader.h"
#include "rmarecordswriter.h"
#include "trans4scif.h"

namespace t4s {

class HBSocket : public Socket {
 private:
  ScifNode sn_;
  RMAWindow recvbuf_;
  RMAWindow sendbuf_;
  RMAId peer_recvbuf_;
  const std::size_t recv_buf_size_;
  std::unique_ptr<RMARecordsWriter> sendrecs_;
  std::unique_ptr<RMARecordsReader> recvrecs_;
  static std::vector<uint8_t> notif;
  int pending_notifs = 0;
  void init();

 public:

  // Construct a connecting node
  explicit HBSocket(uint16_t target_node_id, uint16_t target_port, std::size_t buf_size);

  // Construct a listening node
  explicit HBSocket(uint16_t listening_port, std::size_t buf_size);

  // Contruct from an already connected node
  explicit HBSocket(ScifEpd &epd, std::size_t buf_size);

  // Sends at most len bytes (Streaming semantics)
  std::size_t send(const uint8_t *data, std::size_t data_size) override;
  std::size_t recv(uint8_t *msg_it, std::size_t msg_size) override;

  void waitIn(long timeout) override;
  Buffer getSendBuffer() override;
  std::size_t getBufSize() override { return recv_buf_size_; }
};

}
#endif
