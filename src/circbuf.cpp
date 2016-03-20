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
#include "circbuf.h"
#include "constants.h"

namespace t4s {

std::size_t Circbuf::Write(std::vector<uint8_t>::const_iterator src, std::size_t len) {
  /** Copy the payload */
  off_t wr = get_wr();
  std::size_t len2wr = WrAdvance(len);
  std::copy_n(src, len2wr, base_mem_ + wr);

  return len2wr;
}

std::size_t Circbuf::Read(std::vector<uint8_t>::iterator dest, std::size_t len) {
  off_t rd = get_rd();
  std::size_t len2rd = RdAdvance(len);
  std::copy_n(base_mem_ + rd, len2rd, dest);

  return len2rd;
}

void Circbuf::WrResetChunkHead() {
  /** Reset the chunk head. [The cast is SCIF dependant] */
  uint64_t *head = reinterpret_cast<uint64_t *>(base_mem_ + get_wr());
  *head = 0;
  WrAdvance(CHUNK_HEAD_SIZE);
}

uint64_t Circbuf::RdReadResetChunkHead() {
  /** Read and reset the chunk head. [The cast is SCIF dependant] */
  uint64_t *head = reinterpret_cast<uint64_t *>(base_mem_ + get_rd());
  uint64_t ret = *head;
  *head = 0;
  RdAdvance(CHUNK_HEAD_SIZE);
  return ret;
}

uint64_t Circbuf::WrReadChunkHead() {
  uint64_t ret = *(reinterpret_cast<uint64_t *>(base_mem_ + get_wr()));
  WrAdvance(CHUNK_HEAD_SIZE);
  return ret;
}

}