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
#include "constants.hpp"
#include "rmawindow.hpp"

class RMAWindow_factory
{
private:
	scif_epd_t epd;

public:
	RMAWindow_factory(scif_epd_t epdarg) : epd{epdarg} {}

	/** 
	 * Open a window in the registered memory space of the process. 
	 * len will be rounded up to PAGE_SIZE boundary if necessary.
	 * The prot_flags is formed by OR'ing SCIF_PROT_READ and SCIF_PROT_WRITE. 
	 */
	RMAWindow generate(std::size_t len, int prot_flags = (SCIF_PROT_WRITE | SCIF_PROT_READ))
	{
		return RMAWindow(epd, len, prot_flags);
	}
};