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
#include <system_error>
#include "circbuf.hpp"

/* TODO: Read/write access rights (maybe have it as default only hera) */
Circbuf::Circbuf(std::size_t len, RMAWindow_factory win_a) : 
win_factory{win_a},
win{win_factory.generate(len)},
Virt_circbuf(win.get_off(), win.get_len())
{}

/** TODO: have them again a look also at interface level */
std::size_t Circbuf::write(std::vector<uint8_t>& in)
{
	off_t wr = get_wr();
	uint8_t *base_mem = static_cast<uint8_t *>(win.get_mem());
	std::size_t len2wr = write(in.size());
	std::memcpy(base_mem+wr, in.data(), len2wr);
	in.erase(in.begin(), in.begin()+len2wr);
	return len2wr;
}

/** TODO: have them again a look also at interface level */
std::size_t Circbuf::read(std::vector<uint8_t>& out)
{
	off_t rd = get_rd();
	uint8_t *base_mem = static_cast<uint8_t *>(win.get_mem());
	std::size_t len2rd = read(out.size());
	std::memcpy(out.data(), base_mem+rd, len2rd);
	return len2rd;
}