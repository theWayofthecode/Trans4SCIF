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


#ifndef _CTL_MESSAGES_H_
#define _CTL_MESSAGES_H_

#include <cstdint>
#include <cstddef>
#include <vector>

namespace t4s {

// The type is of known size for any platform in order to avoid corruption
// in case the size of off_t and std::size_t is different for the platforms of the endpoints.
struct RMAId {
  int64_t off;
  uint64_t size;
};

std::vector<uint8_t> PackRMAIdMsg(RMAId osp);

RMAId UnpackRMAIdMsg(std::vector<uint8_t> msg);
#endif

}