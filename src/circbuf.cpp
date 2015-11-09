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

#include <cstddef>
#include <vector>
#include <algorithm>
#include <cstring>
#include <cstdint>
#include <iostream>
#include "circbuf.hpp"
#include "rmawindow.hpp"

std::size_t Circbuf::write(std::vector<uint8_t>& in)
{
	off_t wr = get_wr();
	uint8_t *base_mem = static_cast<uint8_t *>(win.get_mem());
	std::size_t len2wr = write(in.size());
	std::memcpy(base_mem+wr, in.data(), len2wr);
	return len2wr;
}

std::size_t Circbuf::read(std::vector<uint8_t>& out)
{
	off_t rd = get_rd();
	uint8_t *base_mem = static_cast<uint8_t *>(win.get_mem());
	std::size_t len2rd = read(out.size());
	std::memcpy(out.data(), base_mem+rd, len2rd);
	return len2rd;
}
