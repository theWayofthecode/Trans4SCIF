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
#include "rmarecordsreader.h"
#include <cassert>
#include "util.h"

namespace t4s {

  RMARecordsReader::RMARecordsReader(Mmapmem &buf_mem, RMAWindow &wr_win, std::size_t recv_buf_size) :
    buf_mem_{std::move(buf_mem)},
    wr_win_{std::move(wr_win)},
    buf_head_{reinterpret_cast<Record *>(buf_mem_.addr())},
    buf_tail_{buf_head_ + 3},
    wr_head_{reinterpret_cast<Record *>(wr_win_.mem())},
    wr_tail_{wr_head_ + wr_win_.size() / sizeof(Record)},
    wr_idx_{wr_head_},
    buf_idx_{buf_head_},
    recv_buf_size_{recv_buf_size} {
    buf_idx_->start = 0;
    buf_idx_->end = recv_buf_size_;
    buf_idx_++;
  }

  bool RMARecordsReader::read(std::size_t rlen) {
    if (wr_idx_->start+rlen == wr_idx_->end) {
      wr_idx_->start = inval_rec.start;
      wr_idx_->end = inval_rec.end;
      if (++wr_idx_ == wr_tail_)
        wr_idx_ = wr_head_;
      uint64_t new_end = round<CL_SIZE>(buf_idx_->end+rlen);
      buf_idx_->end = new_end;
      // We use new_end to avoid a race condition since buf_idx_->end can be assigned 0 by the
      // sender before the following comparison takes place.
      if (new_end == recv_buf_size_) {
        if (++buf_idx_ == buf_tail_)
          buf_idx_ = buf_head_;
      }
      return true;
    } else {
      buf_idx_->end += rlen;
      wr_idx_->start += rlen;
    }
    assert(buf_idx_->end < recv_buf_size_);
    return false;
  }
}
