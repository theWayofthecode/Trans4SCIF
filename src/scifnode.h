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

namespace t4s {

class ScifNode {
 private:
  ScifEpd epd_;
 public:
  ScifNode() = delete;

//   TODO: Prohibit copying and moving?

//   Construct a connecting node
  ScifNode(uint16_t target_node_id, uint16_t target_port);

//   Construct a listening node
  ScifNode(uint16_t listening_port);

//   Sends synchronously payload.size() bytes. Returns when the payload is delivered
//      to the endpoints receive buffer (of size 4095 bytes). On error it throws a system_error exception
//      TODO: There is deadlock case when peers try to send msgs larger than 4K. With scif_poll can be solved.
//      On the other hand it is used only for controls, but still if one sends constantly msgs and the other one doesn't reveive any, it may block.
  std::size_t SendMsg(std::vector<uint8_t> &payload);

//   Receivs synchronously size bytes.
//   On error it throws a system_error exception
//   TODO: rething about boundary preservation. Shouldn't bytes be discarded?
  std::vector<uint8_t> RecvMsg(std::size_t size);

// Returns an RMAWindow
  RMAWindow CreateRMAWindow(std::size_t len, int prot_flags) { return RMAWindow(epd_.get_epd_t(), len, prot_flags); }

// This method is a simple wrapper arround scif_vwriteto /*
  void WriteMsg(off_t dest, off_t src, std::size_t len);

//  This method writes on a remote location the value val
//  to signify the completion of marked RMA operations
  void SignalPeer(off_t dest, std::uint64_t val);


// Checks whether there is data to be received by RecvMsg (i.e. the call will not block)
  bool HasRecvMsg();
};

}
#endif