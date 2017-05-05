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
#include "rmawindow.h"
#include "mmapmem.h"
#include "record.h"

namespace t4s {

class RMARecordsWriter {
 private:
  RMAWindow const buf_win_;
  Record * const buf_head_;
  Record * const buf_tail_;
  Record volatile *buf_idx_;

  Mmapmem const wr_mem_;
  Record * const wr_head_;
  Record * const wr_tail_;
  Record volatile *wr_idx_;
  std::size_t const recv_buf_size_;
 public:
  explicit RMARecordsWriter(RMAWindow &buf_win, Mmapmem &wr_mem, std::size_t recv_buf_size);

  //Check if there is space in the buffer and also free wr_record slot
  bool canWrite() const {
    return (buf_idx_->end - buf_idx_->start >= CL_SIZE) &&
      (wr_idx_->start == inval_rec.start && wr_idx_->end == inval_rec.end);
  }

  Record getBufRec() const {
    return {buf_idx_->start, buf_idx_->end};
  }

  off_t written(std::size_t wlen);
};

}
