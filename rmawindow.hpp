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

#pragma once
#include <cstddef>
#include <scif.h>

class RMAWindow
{
private:
	scif_epd_t epd;
	void * mem;
	off_t off;
	std::size_t len;

public:
	static const std::size_t PAGE_SIZE = 0x1000;
	static const std::size_t CACHELINE_SIZE = 0x40;

	RMAWindow() = delete;
	
	RMAWindow(const RMAWindow& w) = delete;
	RMAWindow& operator=(const RMAWindow& w) = delete;


	RMAWindow(RMAWindow&& w);
	RMAWindow& operator=(RMAWindow&& w) = delete;

	/* Open a window in the registered memory space of the process of size equal to num_of_pages * PAGE_SIZE.
		The prot_flags is formed by OR'ing SCIF_PROT_READ and SCIF_PROT_WRITE */
	RMAWindow(scif_epd_t epd, int num_of_pages, int prot_flags);

	~RMAWindow();

	void * get_mem() { return mem; }
	off_t get_off() { return off; }
	std::size_t get_len() { return len; }
};