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

#include <scif.h>

namespace t4s {

// Return the version of the library in string
std::string trans4scif_config();

struct Buffer {
  void *base;
  std::size_t space;
};

class Socket {
 public:
  virtual ~Socket() {};
  virtual std::size_t send(const uint8_t *msg, std::size_t msg_size) = 0;
  virtual std::size_t recv(uint8_t *msg, std::size_t msg_size) = 0;
  virtual bool canSend() = 0;
  virtual bool canRecv() = 0;
  virtual Buffer getSendBuffer() = 0;
};

// Construct a connecting node
Socket* connectingSocket(uint16_t target_node_id, uint16_t target_port);

// Construct a listening node
Socket* listeningSocket(uint16_t listening_port);

// Construct a Socket from a connected epd
Socket* epdSocket(scif_epd_t &epd);

}
#endif //_TRANS4SCIF_TRANS4SCIF_H_
