/*
	C Copyright 2015-2016 CERN
	
	This software is distributed under the terms of the 
	GNU General Public Licence version 3 (GPL Version 3), 
	copied verbatim in the file "LICENSE".
	In applying this licence, CERN does not waive 
	the privileges and immunities granted to it by virtue of its status 
	as an Intergovernmental Organization or submit itself to any jurisdiction.
	
	Author: Aram Santogidis <aram.santogidis@cern.ch>
*/

#include "common.hpp"

/**
 * Helper functions
 */
void barrier(ScifNode &sn)
{
	std::vector<uint8_t> msg(1);
	sn.sendMsg(msg);
	sn.recvMsg(1);
}