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
#include <scif.h>
#include "constants.hpp"
#include "util.hpp"

class RMAWindow {
private:
    scif_epd_t epd;
    void *mem;
    off_t off;
    std::size_t len;

public:
    /* Copying is not allowed. */
    RMAWindow(const RMAWindow &w) = delete;

    RMAWindow &operator=(const RMAWindow &w) = delete;

    RMAWindow(RMAWindow &&w);

    RMAWindow &operator=(RMAWindow &&w);

    /**
     * Open a window in the registered memory space of the process.
     * len will be rounded up to PAGE_SIZE boundary if necessary.
     * The prot_flags is formed by OR'ing SCIF_PROT_READ and SCIF_PROT_WRITE
     */
    RMAWindow(scif_epd_t epd, std::size_t len, int prot_flags);

    ~RMAWindow();

    void *get_mem() { return mem; }

    off_t get_off() { return off; }

    std::size_t get_len() { return len; }
};