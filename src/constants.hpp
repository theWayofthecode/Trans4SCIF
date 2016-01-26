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
#include <string>
#include <vector>

#define INT_TO_STR_(i) #i
#define INT_TO_STR(i) INT_TO_STR_(i)
#define __FILE__LINE__ (__FILE__ + std::string(":") + INT_TO_STR(__LINE__))

constexpr std::size_t PAGE_SIZE = 0x1000;
constexpr std::size_t CACHELINE_SIZE = 0x40;

constexpr std::size_t RECV_BUF_SIZE = 2*PAGE_SIZE;

const std::vector<uint8_t> ZEROS_UINT64_T{0, 0, 0, 0, 0, 0, 0, 0};