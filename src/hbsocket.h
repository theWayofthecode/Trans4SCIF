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
#include "virt_circbuf.h"
#include "ctl_messages.h"
#include "circbuf.h"
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
  std::unique_ptr<RMARecordsWriter> sendrecs_;
  std::unique_ptr<RMARecordsReader> recvrecs_;

  void Init();

 public:

  // Construct a connecting node
  explicit HBSocket(uint16_t target_node_id, uint16_t target_port);

  // Construct a listening node
  explicit HBSocket(uint16_t listening_port);

  // Contruct from an already connected node
  explicit HBSocket(ScifEpd &epd);

  // Sends at most len bytes (Streaming semantics)
  std::size_t Send(const uint8_t *msg_it, std::size_t len) override;

  std::size_t Recv(uint8_t *msg_it, std::size_t msg_size) override;
};

}
#endif
