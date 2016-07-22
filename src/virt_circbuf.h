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

#pragma once

#include <cstddef>
#include <iostream>
#include <sys/types.h>
#include "constants.h"
#include "util.h"

namespace t4s {

class VirtCircbuf {
 private:
  off_t base_rmaoff_;
  std::size_t maxlen_;

  std::size_t space_;
  off_t wr_;
  off_t rd_;

 protected:
  off_t get_wr() { return wr_; }

  off_t get_rd() { return rd_; }

 public:
  VirtCircbuf(off_t rmaoff, std::size_t len) :
      base_rmaoff_{rmaoff},
      maxlen_{len},
      space_{len},
      wr_{0},
      rd_{0} { }

//   TODO: copy move constructors: Copy prohibited, move allowed (defined)

  off_t get_wr_rmaoff() { return base_rmaoff_ + wr_; }

  std::size_t get_space() { return space_; }

  bool is_empty() { return space_ == maxlen_; }

  std::size_t WrAdvance(std::size_t len);

  std::size_t RdAdvance(std::size_t len);

  std::size_t WrFullAdvanceAlign(std::size_t wr_len);

  std::size_t RdFullAdvanceAlign(std::size_t rd_len);

  std::size_t WrAlign();

  std::size_t RdAlign();
};

}