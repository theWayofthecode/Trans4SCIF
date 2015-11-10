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

#include <cstdint>
#include <vector>

template<typename inttype>
void inttype_to_vec_le(inttype &in, std::vector<uint8_t> &out)
{
	for (int i = 0; i < sizeof(in); i++, in >>= 8) {
		out.push_back(static_cast<uint8_t>(in));
	}
}

template<typename inttype>
void vec_to_inttype_le(const std::vector<uint8_t> &in, inttype &out)
{
	out = 0;
	int sz = std::min(sizeof(out), in.size());
	auto it = in.cbegin()+sz-1;
	for (int i = 0; i < sz; i++, --it) {
		out = (out << 8) | *it;
	}
}