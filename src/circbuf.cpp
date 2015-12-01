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

#include <vector>
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <system_error>
#include <cstddef>
#include "circbuf.hpp"

/** 
 * 
 * TODO: have them again a look also at interface level
 * return a vector, or accept an (unidirectional)-iterator/pointer start end
 */
std::size_t Circbuf::write(const std::vector<uint8_t>& in)
{
	off_t wr = get_wr();
	uint8_t *base_mem = static_cast<uint8_t *>(win.get_mem());
	std::size_t len2wr = wr_advance(in.size());
	std::copy(in.begin(), in.begin()+len2wr, base_mem+wr);
	return len2wr;
}

std::vector<uint8_t> Circbuf::read(std::size_t len)
{
	off_t rd = get_rd();
	uint8_t *base_mem = static_cast<uint8_t *>(win.get_mem());
	std::size_t len2rd = rd_advance(len);
	return std::vector<uint8_t> (base_mem+rd, base_mem+rd+len2rd);
}