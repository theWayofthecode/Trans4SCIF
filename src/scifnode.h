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
#include <sys/types.h>
#include "scifepd.h"
#include "rmawindow.h"
#include "mmapmem.h"

namespace t4s {

class ScifNode {
 private:
  ScifEpd epd_;
  std::size_t transmission(int(*trans_prim)(scif_epd_t, void *, int, int),
                                     std::vector<uint8_t>::iterator start,
                                     std::vector<uint8_t>::iterator end);

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
  std::size_t SendMsg(std::vector<uint8_t> &payload);

  // Receivs synchronously size bytes.
  // On error it throws a system_error exception
  std::vector<uint8_t> RecvMsg(std::size_t size);

  // Returns an RMAWindow
  RMAWindow CreateRMAWindow(std::size_t len, int prot_flags) {
    return RMAWindow(epd_.get(), len, prot_flags);
  }

  // Returns an mmap object
  Mmapmem createMmapmem(off_t off, std::size_t len, int prot_flags) {
    return Mmapmem(epd_.get(), off, len, prot_flags);
  }

  // This method is a simple wrapper arround scif_vwriteto /*
  void WriteMsg(off_t dest, off_t src, std::size_t len);

  // This method writes on a remote location the value val
  // to signify the completion of marked RMA operations
  void SignalPeer(off_t dest, std::uint64_t val);


  // Checks whether there is data to be received by RecvMsg (i.e. the call will not block)
  bool HasRecvMsg();
};

}
#endif
