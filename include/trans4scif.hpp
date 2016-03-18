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

#ifndef _SOCKET_SOCKET_HPP_
#define _SOCKET_SOCKET_HPP_

#include <vector>
#include <cstdint>
#include "../src/scifnode.hpp"
#include "../src/virt_circbuf.hpp"
#include "../src/circbuf.hpp"

namespace t4s {

std::string trans4scif_version();

class Socket {
 private:
  ScifNode sn_;
  Circbuf recvbuf_;
  std::unique_ptr<Circbuf> sendbuf_;
  /**
   * Remote (peer) receive buffer. It is virtual in the sense
   * that it is not backed up by memory but used only for the logistics.
   */
  std::unique_ptr<VirtCircbuf> rem_recvbuf_;

  void Init();

  void GetRemRecvbufNotifs();

  void UpdateRecvbufSpace();

 public:

  /* Construct a connecting node */
  Socket(uint16_t target_node_id, uint16_t target_port);

  /* Construct a listening node */
  Socket(uint16_t listening_port);

  std::size_t Send(std::vector<uint8_t>::const_iterator msg_it, std::size_t len);

  std::size_t Recv(std::vector<uint8_t>::iterator msg_it, std::size_t msg_size);

  std::vector<uint8_t> Recv();
};

}
#endif