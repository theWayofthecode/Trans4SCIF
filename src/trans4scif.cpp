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

#include <iostream>
#include <algorithm>
#include <scif.h>
#include <cassert>
#include <sstream>
#include <sys/types.h>
#include "hbsocket.h"
#include "trans4scif_config.h"
#include "trans4scif.h"

namespace t4s {

std::string trans4scif_version() {
  std::stringstream ss;
  ss << TRANS4SCIF_VERSION_MAJOR << "." << TRANS4SCIF_VERSION_MINOR;
  return ss.str();
}

// Construct a connecting node
Socket* CreateSocket(uint16_t target_node_id, uint16_t target_port) {
  return new HBSocket(target_node_id, target_port);
}

// Construct a listening node
Socket* CreateSocket(uint16_t listening_port) {
  return new HBSocket(listening_port);
}

}