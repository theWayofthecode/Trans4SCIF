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
#include <assert>
#include "circbuf.hpp"
#include "constants.hpp"


std::size_t Circbuf::write(std::vector<uint8_t>::const_iterator src, std::size_t len)
{
	uint8_t *base_mem = static_cast<uint8_t *>(win.get_mem());

	/** Set to zero the region that signalPeer will write */
	std::copy_n(ZEROS_UINT64_T.cbegin(), ZEROS_UINT64_T.size(), base_mem+get_wr());
	wr_advance(ENTRY_HEAD);
	/** TODO: define a Header size */
	assert(ZEROS_UINT64_T.size() == ENTRY_HEAD);

	/** Copy the payload */
	off_t wr = get_wr();
	std::size_t len2wr = wr_advance(len);
	std::copy_n(src, len2wr, base_mem+wr);
	
	wr_align();
	return len2wr;
}

std::size_t Circbuf::read(std::vector<uint8_t>::iterator dest, std::size_t len)
{
	uint8_t *base_mem = static_cast<uint8_t *>(win.get_mem());
	
	/** Read and reset the chunk size. [This conversion is SCIF dependant] */
	uint64_t *head = reinterpret_cast<uint64_t *>(base_mem+get_rd());
	std::size_t len2rd = *head;
	*head = 0;
	rd_advance(ENTRY_HEAD);

	/** Read the chunk */
	std::size_t len2rd_possible = std::min(len2rd, len);
	std::copy_n(base_mem+get_rd(), len2rd_possible, dest);
	/** assert len2rd == return value of rd_advnace */
	rd_advance(len2rd);

	rd_align();
	return len2rd;
}

void Circbuf::wr_update()
{
	uint8_t *base_mem = static_cast<uint8_t *>(win.get_mem());
	uint64_t *head;
	while (get_space() && *(head = reinterpret_cast<uint64_t *>(base_mem+get_wr()))) {
		write(*head);
	}
}
