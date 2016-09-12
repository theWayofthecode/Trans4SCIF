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
  Mmapmem buf_mem_;
  Record *buf_head_;
  Record *buf_tail_;
  volatile Record *buf_idx_;

  RMAWindow wr_win_;
  Record *wr_head_;
  Record *wr_tail_;
  volatile Record *wr_idx_;

 public:
  explicit RMARecordsReader(Mmapmem &buf_mem, RMAWindow &wr_win_);

  //If there is a valid wr_record
  bool canRead() {
    return (wr_idx_->start != inval_rec.start && wr_idx_->end != inval_rec.end);
  }

  Record getWrRec() {
    return {wr_idx_->start, wr_idx_->end};
  }

  void read(std::size_t rlen);
};

}
