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
#include <memory>
#include "virt_circbuf.hpp"
#include "rmawindow.hpp"

/**
 * TODO: copy prohibited. Think about move 
 */
class Circbuf : public Virt_circbuf
{
private:
	RMAWindow win;

public:
	Circbuf(RMAWindow w) : Virt_circbuf(w.get_off(), w.get_len()), win{std::move(w)} {}

	/* TODO: copy move constructors: Copy prohibited, move allowed (defined) */

	std::size_t write(const std::vector<uint8_t>& in);
	std::vector<uint8_t> read(std::size_t);
};