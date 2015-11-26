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
#include <vector>
#include "virt_circbuf.hpp"
#include "rmawindow_factory.hpp"

/**
 * TODO: copy prohibited. Think about move 
 */
class Circbuf : public Virt_circbuf
{
private:
	RMAWindow_factory win_factory;
	RMAWindow win;

public:
	Circbuf(std::size_t len, RMAWindow_factory win_a);

	/* TODO: copy move constructors: Copy prohibited, move allowed (defined) */

	/* Unhide the inherited write/read methods */
	using Virt_circbuf::write;
	using Virt_circbuf::read;

	std::size_t write(std::vector<uint8_t>& in);
	std::size_t read(std::vector<uint8_t>& out);
};