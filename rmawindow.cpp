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
	winRef.len = num_of_pages * PAGE_SIZE;
	int err = posix_memalign(&winRef.mem, PAGE_SIZE, winRef.len);
	if (err) {
		throw std::system_error(err, std::system_category());
	}
	bzero(winRef.mem, winRef.len);
	winRef.off = scif_register(epd, winRef.mem, winRef.len, 0, prot_flags, 0);
	if (winRef.off == SCIF_REGISTER_FAILED) {
		throw std::system_error(errno, std::system_category());
	}
}

RMAWindow::~RMAWindow()
{
	if (scif_unregister(epd, winRef.off, winRef.len) == -1) {
		std::system_error e(errno, std::system_category());
		std::cerr << "Warning: scif_close: " << e.what() << std::endl;
	}
	free(winRef.mem);
}
