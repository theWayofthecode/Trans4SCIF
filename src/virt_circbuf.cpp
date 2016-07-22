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

#include <algorithm>
#include <cassert>
#include "virt_circbuf.h"

namespace t4s {

std::size_t VirtCircbuf::WrAdvance(std::size_t wr_len) {
  std::size_t len = std::min(std::min(maxlen_ - wr_, space_), wr_len);
  wr_ = (wr_ + len) % maxlen_;
  space_ -= len;
  return len;
}

std::size_t VirtCircbuf::RdAdvance(std::size_t rd_len) {
  std::size_t len = std::min(std::min(maxlen_ - rd_, maxlen_ - space_), rd_len);
  rd_ = (rd_ + len) % maxlen_;
  space_ += len;
  return len;
}

std::size_t VirtCircbuf::WrAlign() {
  off_t new_wr = ROUND_TO_BOUNDARY(wr_, CACHELINE_SIZE);
  std::size_t to_advance = new_wr - wr_;
  space_ -= to_advance;
  wr_ = new_wr % maxlen_;
  return to_advance;
}

std::size_t VirtCircbuf::RdAlign() {
  off_t new_rd = ROUND_TO_BOUNDARY(rd_, CACHELINE_SIZE);
  std::size_t to_advance = new_rd - rd_;
  space_ += new_rd - rd_;
  rd_ = new_rd % maxlen_;
  return to_advance;
}

std::size_t VirtCircbuf::WrFullAdvanceAlign(std::size_t wr_len) {
  auto advanced = WrAdvance(wr_len);
  if (advanced < wr_len) //wrap around case
    advanced += WrAdvance(wr_len - advanced);
  assert(advanced == wr_len);
  advanced += WrAlign();
  return advanced;
}

std::size_t VirtCircbuf::RdFullAdvanceAlign(std::size_t rd_len) {
  auto advanced = RdAdvance(rd_len);
  if (advanced < rd_len) //wrap around case
    advanced += RdAdvance(rd_len - advanced);
  assert(advanced == rd_len);
  advanced += RdAlign();
  return advanced;
}

}