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

#ifndef _UTIL_UTIL_H_
#define _UTIL_UTIL_H_

#include <cstdint>
#include <algorithm>
#include <vector>
#include <thread>

#define INT_TO_STR_(i) #i
#define INT_TO_STR(i) INT_TO_STR_(i)
#define __FILE__LINE__ (__FILE__ + std::string(":") + INT_TO_STR(__LINE__))

// args must be unsigned integers !!!
#define ROUND_TO_BOUNDARY(s, b) ((s+b-1) & (-b))

namespace t4s {

template<typename inttype>
void inttype_to_vec_le(inttype in, std::vector<uint8_t> &out) {
  for (int i = 0; i < sizeof(in); i++, in >>= 8) {
    out.push_back(in & 0xff);
  }
}

template<typename inttype>
void vec_to_inttype_le(const std::vector<uint8_t> &in, inttype &out) {
  out = 0;
  for (auto it = in.cbegin() + std::min(sizeof(out), in.size()) - 1;
       it >= in.cbegin();
       --it) {
    out = (out << 8) | *it;
  }
}

void scaled_sleep(int v, int s, int m);

}
#endif

