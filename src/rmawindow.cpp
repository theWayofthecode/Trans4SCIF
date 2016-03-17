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
#include <cstdlib>
#include <system_error>
#include <scif.h>
#include <iostream>
#include <cstring>

#include "rmawindow.hpp"

RMAWindow::RMAWindow(scif_epd_t epd, std::size_t len, int prot_flags)
{
	this->epd = epd;
	this->len = ROUND_TO_BOUNDARY(len, PAGE_SIZE);
	int err = posix_memalign(&mem, PAGE_SIZE, this->len);
	if (err) {
		throw std::system_error(err, std::system_category(), __FILE__LINE__);
	}
	bzero(mem, this->len);
	off = scif_register(epd, mem, this->len, 0, prot_flags, 0);
	if (off == SCIF_REGISTER_FAILED) {
		throw std::system_error(errno, std::system_category(), __FILE__LINE__);
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
	w.off = 0;
	w.len = 0;
}

RMAWindow& RMAWindow::operator=(RMAWindow&& w)
{
	epd = w.epd;
	mem = w.mem;
	off = w.off;
	len = w.len;
	w.epd = -1;
	w.mem = nullptr;
	w.off = 0;
	w.len = 0;
	return *this;
}

RMAWindow::~RMAWindow()
{
	if (mem) {
		free(mem);
	}
	if (epd != -1) {
		if (scif_unregister(epd, off, len) == -1) {
			std::system_error e(errno, std::system_category(), __FILE__LINE__);
			std::cerr << "Warning: scif_unregister: " << e.what() << __FILE__LINE__ << std::endl;
		}
	}
}
