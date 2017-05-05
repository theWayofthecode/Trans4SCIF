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
#include <memory>

namespace t4s {

// Default internal buffer size
constexpr std::size_t BUF_SIZE = 0x1000000;

// Return the version of the library
std::string trans4scif_config();

// TODO: use memory block representation for send and receive
// void* to uint8_t* ?
struct Blk {
  void *base;
  std::size_t space;
};

class Socket {
 public:
  // Construct a connecting node
  explicit Socket(uint16_t target_node_id, uint16_t target_port, std::size_t buf_size = BUF_SIZE);

  // Construct a listening node
  explicit Socket(uint16_t listening_port, std::size_t buf_size = BUF_SIZE);

  // Contruct from an already connected node
  explicit Socket(scif_epd_t epd, std::size_t buf_size = BUF_SIZE);

  // Copy is prohibited
  Socket(Socket const &s) = delete;
  Socket &operator=(Socket const &s) = delete;
  
  ~Socket();

  // Send up to data_size number of bytes from the memory region pointed by data.
  // Return the number of bytes actually sent. This method does not block.
  std::size_t send(uint8_t *data, std::size_t data_size);

  // Receive (by copying) up to data_size number of bytes to the memory region pointed by data.
  // Return the number of bytes actually received. This method does not block.
  std::size_t recv(uint8_t *data, std::size_t data_size);

  // Block up to timeout seconds till there is available data to be received.
  // If timeout occurs, and std::runtime_error exception is thrown.
  void waitIn(long timeout);

  // Get the internal Send buffer details.
  // This method must invoked for each use.
  Blk getSendBuffer () const ;

 private:
  // Pimpl
  class SockState;
  std::unique_ptr<SockState> const s;
};

}
#endif //_TRANS4SCIF_TRANS4SCIF_H_
