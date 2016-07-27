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

#include <vector>
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <system_error>
#include <cstddef>
#include <cassert>
#include <cstring>
#include "circbuf.h"
#include "trans4scif_config.h"

namespace t4s {

std::size_t Circbuf::Write(const uint8_t *src, std::size_t len) {
  off_t wr = get_wr();
  std::size_t len2wr = WrAdvance(len);
  std::copy_n(src, len2wr, base_mem_ + wr);
  return len2wr;
}

std::size_t Circbuf::Read(uint8_t *dest, std::size_t len) {
  std::size_t allrd = len;
  auto read = [this, &dest, &allrd]() {
    off_t rd = get_rd();
    std::size_t l = RdAdvance(allrd);
    std::copy_n(base_mem_ + rd, l, dest);
    std::memset(base_mem_ + rd, 0, l);
    dest += l;
    allrd -= l;
  };
  read();
  //Wrap around case
  if (allrd)
    read();
  return len - allrd;
}

void Circbuf::WrResetChunkHead() {
  // Reset the chunk head. [The cast is SCIF dependant]
  uint64_t *head = reinterpret_cast<uint64_t *>(base_mem_ + get_wr());
  *head = 0;
  WrAdvance(CHUNK_HEAD_SIZE);
}

uint64_t Circbuf::RdReadResetChunkHead() {
  // Read and reset the chunk head. [The cast is SCIF dependant]
  uint64_t *head = reinterpret_cast<uint64_t *>(base_mem_ + get_rd());
  uint64_t ret = *head;
  *head = 0;
  RdAdvance(CHUNK_HEAD_SIZE);
  return ret;
}

uint64_t Circbuf::WrReadChunkHead() {
  uint64_t ret = *(reinterpret_cast<uint64_t *>(base_mem_ + get_wr()));
  return ret;
}

}