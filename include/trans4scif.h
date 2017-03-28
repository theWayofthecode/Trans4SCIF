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
#include <future>

namespace t4s {

// Default internal buffer size
constexpr std::size_t BUF_SIZE = 0x1000000;

// Return the version of the library
std::string trans4scif_config();

struct Buffer {
  void *base;
  std::size_t space;
};

class Socket {
 public:
  virtual ~Socket() {};
  
  // Send up to data_size number of bytes from the memory region pointed by data.
  // Return the number of bytes actually sent. This method does not block.
  virtual std::size_t send(const uint8_t *data, std::size_t data_size) = 0;

  // Receive (by copying) up to data_size number of bytes to the memory region pointed by data.
  // Return the number of bytes actually received. This method does not block.
  virtual std::size_t recv(uint8_t *data, std::size_t data_size) = 0;

  // Block up to timeout seconds till there is available data to be received.
  // If timeout occurs, and std::runtime_error exception is thrown.
  virtual void waitIn(long timeout) = 0;

  // Get the internal Send buffer details.
  // This method must invoked for each use.
  virtual Buffer getSendBuffer() = 0;

  // Get the internal buffer size.
  // Note: recvbuf_size == sendbuf_size
  virtual std::size_t getBufSize() = 0;
};

// Construct a connecting node
Socket* connectingSocket(uint16_t target_node_id, uint16_t target_port, std::size_t buf_size = BUF_SIZE);

// Construct a listening node
Socket* listeningSocket(uint16_t listening_port, std::size_t buf_size = BUF_SIZE);

// Construct a Socket from a connected epd
Socket* epdSocket(scif_epd_t epd, std::size_t buf_size = BUF_SIZE);

// Construct a Socket from a connected epd, asynchronously
std::future<Socket *> epdSocketAsync(scif_epd_t epd, std::size_t buf_size = BUF_SIZE);

}
#endif //_TRANS4SCIF_TRANS4SCIF_H_
