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

#pragma once

namespace t4s {

struct Record {
  uint64_t start;
  uint64_t end;
};

const Record inval_rec{static_cast<uint64_t>(-1), static_cast<uint64_t>(-1)};

}
