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

#include <cstdint>
#include <cstddef>
#include <sys/types.h>
#include "trans4scif_config.h"

namespace t4s {

struct Record {
  uint64_t start;
  uint64_t end;
};

class RMARecords {
 private:
  //Receiver records
  Record *buf_head_;
  Record *buf_tail_;
  volatile Record *buf_idx_;

  //Sender records
  off_t wr_off_;
  Record *wr_head_;
  Record *wr_tail_;
  volatile Record *wr_idx_;

 public:
  explicit RMARecords(void *buf_base, void *wr_base, off_t roff); 

  //If there is a valid wr_record
  bool canRead() {
    return (wr_idx_->start < wr_idx_->end);
  }

  //Check if there is space in the buffer and also free wr_record slot
  bool canWrite() {
    return (buf_idx_->end - buf_idx_->start >= CACHELINE_SIZE) &&
      (wr_idx_->start == 0 && wr_idx_->end == 0);
  }

  Record getBufRec() {
    return {buf_idx_->start, buf_idx_->end};
  }

  Record getWrRec() {
    return {wr_idx_->start, wr_idx_->end};
  }

  void read(std::size_t rlen);
  off_t written(std::size_t wlen);
};

}
