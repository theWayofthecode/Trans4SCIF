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

#include "util.h"
#include "ctl_messages.h"

namespace t4s {

std::vector<uint8_t> PackRMAIdMsg(RMAId id) {
  std::vector<uint8_t> msg;
  inttype_to_vec_le(id.off, msg);
  inttype_to_vec_le(id.size, msg);
  return msg;
}

RMAId UnpackRMAIdMsg(std::vector<uint8_t> msg) {
  /* assert msg.size() >= sizeof(off_size_pair) */
  RMAId id;
  vec_to_inttype_le(msg, id.off);
  msg.erase(msg.begin(), msg.begin() + sizeof(id.off));
  vec_to_inttype_le(msg, id.size);
  return id;
}

}