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

#ifndef _TRANS4SCIF_TRANS4SCIF_H_
#define _TRANS4SCIF_TRANS4SCIF_H_

#include <vector>
#include <future>
#include <scif.h>

namespace t4s {

// Return the version of the library in string
std::string trans4scif_version();

class Socket {
 public:
  virtual ~Socket() {};
  virtual std::size_t Send(std::vector<uint8_t>::const_iterator msg_it, std::size_t len) = 0;
  virtual std::size_t Recv(std::vector<uint8_t>::iterator msg_it, std::size_t msg_size) = 0;
  virtual std::vector<uint8_t> Recv() = 0;
};

// Construct a connecting node
Socket* Connect(uint16_t target_node_id, uint16_t target_port);

// Construct a listening node
Socket* Listen(uint16_t listening_port);

// Get a Socket from a connected scif_epd_t
Socket* SocketFromEpd(scif_epd_t epd);
std::future<Socket*> SocketFromEpdAsync(scif_epd_t epd);

}
#endif //_TRANS4SCIF_TRANS4SCIF_H_
