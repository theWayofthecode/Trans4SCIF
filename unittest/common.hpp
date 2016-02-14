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

#pragma once

#include "../src/scifnode.hpp"

#ifdef XEONPHI
#define ADDR 0, 5555
#else
#define ADDR 5555
#endif

/**
 * Helper functions
 */
void barrier(ScifNode &sn);