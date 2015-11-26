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

/**
 * The type is of known size for any platform in order to avoid corruption
 * in case the size of off_t and std::size_t is different for the platforms of the endpoints.
 */
struct RMA_id {
	int64_t off;
	uint64_t size;
};

std::vector<uint8_t> pack_RMA_id_msg(RMA_id osp);
RMA_id unpack_RMA_id_msg(std::vector<uint8_t> msg);