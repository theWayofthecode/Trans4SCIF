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
#include "constants.hpp"
#include "util.hpp"

class Virt_circbuf {
private:
    off_t base_rmaoff;
    std::size_t maxlen;

    std::size_t space;
    off_t wr;
    off_t rd;

protected:
    off_t get_wr() { return wr; }

    off_t get_rd() { return rd; }

public:
    Virt_circbuf(off_t rmaoff, std::size_t len) :
            base_rmaoff{rmaoff},
            maxlen{len},
            space{len},
            wr{0},
            rd{0} { }

    /* TODO: copy move constructors: Copy prohibited, move allowed (defined) */

    off_t get_wr_rmaoff() { return base_rmaoff + wr; }

    std::size_t get_space() { return space; }

    bool is_empty() { return space == maxlen; }

    std::size_t wr_advance(std::size_t len);

    std::size_t rd_advance(std::size_t len);

    void wr_align();

    void rd_align();
};