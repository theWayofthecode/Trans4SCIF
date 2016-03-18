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
#include "virt_circbuf.hpp"

std::size_t Virt_circbuf::wr_advance(std::size_t wr_len) {
    std::size_t len = std::min(std::min(maxlen - wr, space), wr_len);
    wr = (wr + len) % maxlen;
    space -= len;
    return len;
}

std::size_t Virt_circbuf::rd_advance(std::size_t rd_len) {
    std::size_t len = std::min(std::min(maxlen - rd, maxlen - space), rd_len);
    rd = (rd + len) % maxlen;
    space += len;
    return len;
}

void Virt_circbuf::wr_align() {
    off_t new_wr = ROUND_TO_BOUNDARY(wr, CACHELINE_SIZE);
    space -= new_wr - wr;
    wr = new_wr % maxlen;
}

void Virt_circbuf::rd_align() {
    off_t new_rd = ROUND_TO_BOUNDARY(rd, CACHELINE_SIZE);
    space += new_rd - rd;
    rd = new_rd % maxlen;
}
