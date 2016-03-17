/*
	Copyright (c) 2015-2016 CERN
	
	This software is distributed under the terms of the 
	GNU Lesser General Public Licence version 3 (LGPLv3),
	copied verbatim in the file "LICENSE".
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
#include <cassert>
#include "circbuf.hpp"
#include "constants.hpp"


std::size_t Circbuf::write(std::vector<uint8_t>::const_iterator src, std::size_t len)
{
	/** Copy the payload */
	off_t wr = get_wr();
	std::size_t len2wr = wr_advance(len);
	std::copy_n(src, len2wr, base_mem+wr);
	
	return len2wr;
}

std::size_t Circbuf::read(std::vector<uint8_t>::iterator dest, std::size_t len)
{
	off_t rd = get_rd();
	std::size_t len2rd = rd_advance(len);
	std::copy_n(base_mem+rd, len2rd, dest);

	return len2rd;
}

void Circbuf::wr_reset_chunk_head()
{
	/** Reset the chunk head. [The cast is SCIF dependant] */
	uint64_t *head = reinterpret_cast<uint64_t *>(base_mem+get_wr());
	*head = 0;
	wr_advance(CHUNK_HEAD_SIZE);
}

uint64_t Circbuf::rd_read_reset_chunk_head()
{
	/** Read and reset the chunk head. [The cast is SCIF dependant] */
	uint64_t *head = reinterpret_cast<uint64_t *>(base_mem+get_rd());
	uint64_t ret = *head;
	*head = 0;
	rd_advance(CHUNK_HEAD_SIZE);
	return ret;
}

uint64_t Circbuf::wr_read_chunk_head()
{
	uint64_t ret = *(reinterpret_cast<uint64_t *>(base_mem+get_wr()));
	wr_advance(CHUNK_HEAD_SIZE);
	return ret;
}