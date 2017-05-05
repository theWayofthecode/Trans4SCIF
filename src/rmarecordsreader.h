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
#include <iostream>
#include <cstddef>
#include <memory>
#include <sys/types.h>
#include "trans4scif_config.h"
#include "mmapmem.h"
#include "rmawindow.h"
#include "record.h"

namespace t4s {

class RMARecordsReader {
 private:
  Mmapmem const buf_mem_;
  Record * const buf_head_;
  Record * const buf_tail_;
  Record volatile * buf_idx_;

  RMAWindow const wr_win_;
  Record * const wr_head_;
  Record * const wr_tail_;
  Record volatile * wr_idx_;

  std::size_t const recv_buf_size_;

 public:
  explicit RMARecordsReader(Mmapmem &buf_mem, RMAWindow &wr_win_, std::size_t recv_buf_size);

  //If there is a valid wr_record
  bool canRead() const {
    return (wr_idx_->start != inval_rec.start && wr_idx_->end != inval_rec.end);
  }

  Record getWrRec() const {
    return {wr_idx_->start, wr_idx_->end};
  }

  // Return true if a full WrRecord was consumed
  bool read(std::size_t rlen);
};

}
