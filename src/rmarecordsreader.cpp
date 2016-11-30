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
#include "rmarecordsreader.h"
#include "util.h"

namespace t4s {

  RMARecordsReader::RMARecordsReader(Mmapmem &buf_mem, RMAWindow &wr_win) :
    buf_mem_{std::move(buf_mem)},
    wr_win_{std::move(wr_win)},
    buf_head_{reinterpret_cast<Record *>(buf_mem_.get_addr())},
    wr_head_{reinterpret_cast<Record *>(wr_win_.get_mem())} {

    buf_tail_ = buf_head_ + 3;
    buf_idx_ = buf_head_;
    wr_tail_ = wr_head_ + wr_win_.get_len() / sizeof(Record);
    wr_idx_ = wr_head_;
    buf_idx_->start = 0;
    buf_idx_->end = RECV_BUF_SIZE;
  }

  bool RMARecordsReader::read(std::size_t rlen) {
    volatile Record *next_buf = buf_idx_ + 1;
    if (next_buf == buf_tail_)
      next_buf = buf_head_;
    next_buf->end += rlen;
    wr_idx_->start += rlen;
    assert(wr_idx_->start <= wr_idx_->end);
    if (wr_idx_->start == wr_idx_->end) {
      wr_idx_->start = inval_rec.start;
      wr_idx_->end = inval_rec.end;
      if (++wr_idx_ == wr_tail_)
        wr_idx_ = wr_head_;
      next_buf->end = ROUND_TO_BOUNDARY(next_buf->end, CACHELINE_SIZE);
      if (next_buf->end == RECV_BUF_SIZE)
        buf_idx_ = next_buf;
      return true;
    }
    assert(next_buf->end < RECV_BUF_SIZE);
    return false;
  }
}
