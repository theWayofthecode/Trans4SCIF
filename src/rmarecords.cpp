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
#include "rmarecords.h"
#include "util.h"

namespace t4s {

  RMARecords::RMARecords(void *buf_base, void *wr_base, off_t wroff) :
    buf_head_{reinterpret_cast<Record *>(buf_base)},
    wr_head_{reinterpret_cast<Record *>(wr_base)},
    wr_off_{wroff} {
    //Init receivers records space
    buf_tail_ = buf_head_ + 2;
    buf_idx_ = buf_head_;
    buf_idx_->start = 0;
    buf_idx_->end = RECV_BUF_SIZE;

    //Init senders records space
    wr_tail_ = wr_head_ + WR_WIN_SIZE * PAGE_SIZE / sizeof(Record);
    wr_idx_ = wr_head_;
  }

  void RMARecords::read(std::size_t rlen) {
    volatile Record *next_buf = buf_idx_ + 1;
    if (next_buf == buf_tail_)
      next_buf = buf_head_;
    next_buf->end += rlen;
    wr_idx_->start += rlen;
    assert(wr_idx_->start <= wr_idx_->end);
    if (wr_idx_->start == wr_idx_->end) {
      wr_idx_->start = 0;
      wr_idx_->end = 0;
      if (++wr_idx_ == wr_tail_)
        wr_idx_ = wr_head_;
      next_buf->end = ROUND_TO_BOUNDARY(next_buf->end, CACHELINE_SIZE);
    }
    //OPTIMIZATION:Maybe this check can be done inside the above if,
    //since a single transaction cannot wrap around the buffer
    if (next_buf->end == RECV_BUF_SIZE)
      buf_idx_ = next_buf;
  }

  off_t RMARecords::written(std::size_t wlen) {
    assert(wr_idx_->start == 0 && wr_idx_->end == 0);
    //add wr_record
    wr_idx_->start = buf_idx_->start;
    wr_idx_->end = 0;
    //The sig_off will be used by scif_fence_signal
    std::ptrdiff_t dist_in_bytes = reinterpret_cast<volatile uint8_t *>(&wr_idx_->end) -
      reinterpret_cast<volatile uint8_t *>(wr_head_);
    off_t sig_off = wr_off_ + dist_in_bytes;

    if (++wr_idx_ == wr_tail_)
      wr_idx_ = wr_head_;

    //update buf record
    buf_idx_->start += ROUND_TO_BOUNDARY(wlen, CACHELINE_SIZE);
    if (buf_idx_->start == buf_idx_->end) {
      buf_idx_->start = 0;
      buf_idx_->end = 0;
      if (++buf_idx_ == buf_tail_)
        buf_idx_ = buf_head_;
    }
    return sig_off;
  }
}
