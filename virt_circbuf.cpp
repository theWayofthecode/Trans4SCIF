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

#include <algorithm>
#include "virt_circbuf.hpp"
#include "rmawindow.hpp"


std::size_t Virt_circbuf::write(std::size_t wr_len)
{
	/* Round up to cacheline size */
	std::size_t len = ((wr_len+CACHELINE_SIZE-1)/CACHELINE_SIZE)*CACHELINE_SIZE;
	while (space && len) {
		std::size_t minlen = std::min(std::min(maxlen-wr, space), len);
		((wr+minlen) == maxlen) ? wr = 0 : wr += minlen;
		space -= minlen;
		len -= minlen;
	}
	return wr_len-len;
}

std::size_t Virt_circbuf::read(std::size_t rd_len)
{
	/* Round up to cacheline size */
	int clsize = 0x40;
	std::size_t len = ((rd_len+CACHELINE_SIZE-1)/CACHELINE_SIZE)*CACHELINE_SIZE;
	while ((space < maxlen) && len) {
		std::size_t minlen = std::min(std::min(maxlen-rd, maxlen-space), len);
		((rd+minlen) == maxlen) ? rd = 0 : rd += minlen;
		space += minlen;
		len -= minlen;
	}
	return rd_len-len;
}