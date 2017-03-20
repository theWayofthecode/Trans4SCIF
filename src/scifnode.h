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

#ifndef _SCIFNODE_H_
#define _SCIFNODE_H_

#include <system_error>
#include <cstdint>
#include <vector>
#include <memory>
#include <iostream>
#include <sys/types.h>
#include "scifepd.h"
#include "rmawindow.h"
#include "mmapmem.h"

namespace t4s {

class ScifNode {
 private:
  ScifEpd epd_;

 public:
  // Construct a connecting node
  ScifNode(uint16_t target_node_id, uint16_t target_port);

  // Construct a listening node
  ScifNode(uint16_t listening_port);

  // Construct ScifNode from connected ScifEpd
  ScifNode(ScifEpd &epd) : epd_(std::move(epd)) {};

  // Move constructor and assignment
  ScifNode(ScifNode &&sn) : epd_(std::move(sn.epd_)) {};
  ScifNode &operator=(ScifNode &&sn) { epd_= std::move(sn.epd_); return *this; };

  // Copy is prohibited
  ScifNode(const ScifNode &sn) = delete;
  ScifNode &operator=(const ScifNode &sn) = delete;

  // Sends synchronously payload.size() bytes. Returns when the payload is delivered
  // to the endpoints receive buffer (of size 4095 bytes). On error it throws a system_error exception
  // On the other hand it is used only for controls, but still if one sends constantly msgs and the other one doesn't reveive any, it may block.
  std::size_t sendMsg(std::vector<uint8_t> &payload);

  // Receivs synchronously size bytes.
  // On error it throws a system_error exception
  std::vector<uint8_t> recvMsg(std::size_t size);

  int send(void *data, int len) { return scif_send(epd_.get(), data, len, 0); }
  int recv(void *data, int len) { return scif_recv(epd_.get(), data, len, 0); }

  // Returns an RMAWindow
  RMAWindow createRMAWindow(std::size_t len, int prot_flags) {
    return RMAWindow(epd_.get(), len, prot_flags);
  }

  // Returns an mmap object
  Mmapmem createMmapmem(off_t off, std::size_t len, int prot_flags) {
    return Mmapmem(epd_.get(), off, len, prot_flags);
  }

  // This method is a simple wrapper arround scif_vwriteto /*
  void writeMsg(off_t dest, off_t src, std::size_t len);

  // This method writes on a remote location the value val
  // to signify the completion of marked RMA operations
  void signalPeer(off_t dest, std::uint64_t val);


  // Blocks till data can be sent/received or timeout (in ms) has expired
  bool canRecvMsg(long timeout);
  bool canSendMsg(long timeout);
};

}
#endif
