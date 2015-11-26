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
#include "util.hpp"


std::size_t Virt_circbuf::write(std::size_t wr_len)
{
	std::size_t plen = std::min(std::min(maxlen-wr, space), wr_len);
	std::size_t aligned_len = ROUND_TO_BOUNDARY(plen, CACHELINE_SIZE);
	wr = (wr+aligned_len) % maxlen;
	space -= aligned_len;
	return plen;
}

std::size_t Virt_circbuf::read(std::size_t rd_len)
{
	std::size_t plen = std::min(std::min(maxlen-rd, maxlen-space), rd_len);
	std::size_t aligned_len = ROUND_TO_BOUNDARY(plen, CACHELINE_SIZE);
	rd = (rd+aligned_len) % maxlen;
	space += aligned_len;
	return plen;
}