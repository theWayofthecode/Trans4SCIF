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

#include <cassert>
#include <memory>
#include <iostream>
#include "rmarecordswriter.h"
#include "util.h"

namespace t4s {

  RMARecordsWriter::RMARecordsWriter(RMAWindow &buf_win, Mmapmem &wr_mem, std::size_t recv_buf_size) :
    buf_win_{std::move(buf_win)},
    wr_mem_{std::move(wr_mem)},
    buf_head_{reinterpret_cast<Record *>(buf_win_.get_mem())},
    wr_head_{reinterpret_cast<Record *>(wr_mem_.get_addr())},
    recv_buf_size_(recv_buf_size) {

    buf_tail_ = buf_head_ + 3;
    buf_idx_ = buf_head_;
    wr_tail_ = wr_head_ + wr_mem_.get_len() / sizeof(Record);
    wr_idx_ = wr_head_;
  }

  off_t RMARecordsWriter::written(std::size_t wlen) {
    assert(wr_idx_->start == inval_rec.start && wr_idx_->end == inval_rec.end);
    //add wr_record
    wr_idx_->start = buf_idx_->start;
    wr_idx_->end = inval_rec.end;
    //The sig_off will be used by scif_fence_signal
    std::ptrdiff_t dist_in_bytes = reinterpret_cast<volatile uint8_t *>(&wr_idx_->end) -
      reinterpret_cast<volatile uint8_t *>(wr_head_);
    off_t sig_off = wr_mem_.get_off() + dist_in_bytes;

    if (++wr_idx_ == wr_tail_)
      wr_idx_ = wr_head_;

    //update buf record
    buf_idx_->start += ROUND_TO_BOUNDARY(wlen, CACHELINE_SIZE);
    if (buf_idx_->start == recv_buf_size_) {
      buf_idx_->start = 0;
      buf_idx_->end = 0;
      if (++buf_idx_ == buf_tail_)
        buf_idx_ = buf_head_;
    }
    return sig_off;
  }
}
