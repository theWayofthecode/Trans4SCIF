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

class Virt_circbuf
{
private:
	off_t base_rmaoff;
	std::size_t maxlen;

	std::size_t space;
	off_t wr;
	off_t rd;

public:
	Virt_circbuf(off_t rmaoff, std::size_t len) :
		base_rmaoff{rmaoff},
		maxlen{len},
		space{len},
		wr{0},
		rd{0} {}

	/* copy move constructors: Copy prohibited, move allowed (defined) */

	off_t get_wr_rmaoff() { return base_rmaoff+wr; }

	std::size_t get_space() { return space; }

	std::size_t write(std::size_t len);

	std::size_t read(std::size_t len);
};