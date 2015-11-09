/*
	© Copyright 2015 CERN
	
	This software is distributed under the terms of the 
	GNU General Public Licence version 3 (GPL Version 3), 
	copied verbatim in the file “LICENSE”.
	In applying this licence, CERN does not waive 
	the privileges and immunities granted to it by virtue of its status 
	as an Intergovernmental Organization or submit itself to any jurisdiction.
	
	Author: Aram Santogidis <aram.santogidis@cern.ch>
*/
#include <cstdlib>
#include <system_error>
#include <new>     
#include <scif.h>
#include <iostream>
#include <cstring>

#include "rmawindow.hpp"

RMAWindow::RMAWindow(scif_epd_t epd, int num_of_pages, int prot_flags)
{
	this->epd = epd;
	len = num_of_pages * PAGE_SIZE;
	int err = posix_memalign(&mem, PAGE_SIZE, len);
	if (err) {
		throw std::system_error(err, std::system_category());
	}
	bzero(mem, len);
	off = scif_register(epd, mem, len, 0, prot_flags, 0);
	if (off == SCIF_REGISTER_FAILED) {
		throw std::system_error(errno, std::system_category());
	}
}

RMAWindow::RMAWindow(RMAWindow&& w) 
			 : epd{w.epd},
			   mem{w.mem},
			   off{w.off},
			   len{w.len}
{
	w.epd = -1;
	w.mem = nullptr;
	w.off = -1;
	w.len = -1;
}

/* Move assignment */

RMAWindow::~RMAWindow()
{
	if (epd != -1) {
		if (scif_unregister(epd, off, len) == -1) {
			std::system_error e(errno, std::system_category());
			std::cerr << "Warning: scif_close: " << e.what() << std::endl;
		}
	}
	if (mem) {
		free(mem);
	}
}
