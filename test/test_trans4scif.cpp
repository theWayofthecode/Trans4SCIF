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

#include <system_error>
#include <cassert>
#include <algorithm>
#include <future>
#include <random>
#include <scif.h>
#include "catch.hpp"
#include "trans4scif.h"
#include "test_common.h"
#include "scifepd.h"
#include "../src/trans4scif_config.h"


scif_epd_t plain_scif_connect(int target_node_id, int target_port) {
  scif_epd_t epd = scif_open();
  if(epd == SCIF_OPEN_FAILED)
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);
  struct scif_portID target_addr;
  target_addr.node = target_node_id;
  target_addr.port = target_port;
  if (scif_connect(epd, &target_addr) == -1)
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);
  return epd;
}

scif_epd_t plain_scif_listen(int listening_port) {
  scif_epd_t l = scif_open();
  if(l == SCIF_OPEN_FAILED)
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);

  if (scif_bind(l, listening_port) == -1)
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);

//   listen (backlog = 1)
  if (scif_listen(l, 1) == -1)
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);

//   accept
  scif_epd_t acc_epd_t;
  struct scif_portID peer_addr;
  if (scif_accept(l, &peer_addr, &acc_epd_t, SCIF_ACCEPT_SYNC) == -1)
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);
  if (scif_close(l) == -1)
    throw std::system_error(errno, std::system_category(), __FILE__LINE__);
  return acc_epd_t;
}

TEST_CASE("Test version", "[trans4scif]") {
  std::cerr << t4s::trans4scif_config();
}

TEST_CASE("Send one byte", "[trans4scif]") {
  auto s_pair = MakeConnectedNodes<std::shared_ptr<t4s::Socket>>(t4s::Listen, t4s::Connect);
  uint8_t s = 'x';
  uint8_t r = 0;
  REQUIRE(1 == s_pair[0]->Send(&s, 1));
  REQUIRE(1 == s_pair[1]->Recv(&r, 1));
  REQUIRE(r == 'x');
}

TEST_CASE("Split wrap around", "[trans4scif]") {
  auto s_pair = MakeConnectedNodes<std::shared_ptr < t4s::Socket>>
  (t4s::Listen, t4s::Connect);

  //  wrap around the recv buffer
  std::size_t chunk_size = 128;
  uint8_t fill_value = 0xA;
  std::vector <uint8_t> send_msg(t4s::RECV_BUF_SIZE);
  std::vector <uint8_t> recv_msg(t4s::RECV_BUF_SIZE);
  std::fill_n(send_msg.begin(), t4s::RECV_BUF_SIZE, fill_value);
  std::size_t almost_filled_sz = t4s::RECV_BUF_SIZE - 128;

  REQUIRE( almost_filled_sz == s_pair[0]->Send(send_msg.data(), almost_filled_sz) );
  REQUIRE( almost_filled_sz == s_pair[1]->Recv(recv_msg.data(), almost_filled_sz) );
  REQUIRE( std::all_of(recv_msg.begin(), recv_msg.begin() + almost_filled_sz,
           [fill_value](uint8_t v){return v == fill_value;}) );
  REQUIRE( chunk_size == s_pair[0]->Send(send_msg.data(), chunk_size) );
  REQUIRE( chunk_size == s_pair[1]->Recv(recv_msg.data(), chunk_size) );
  REQUIRE( std::all_of(recv_msg.begin(), recv_msg.begin() + chunk_size,
           [fill_value](uint8_t v){return v == fill_value;}) );
}

TEST_CASE("Send and Recv 0 bytes", "[trans4scif]") {
  auto s_pair = MakeConnectedNodes<std::shared_ptr < t4s::Socket>>
  (t4s::Listen, t4s::Connect);
  REQUIRE(0 == s_pair[0]->Send(nullptr, 0));
  REQUIRE(0 == s_pair[1]->Recv(nullptr, 0));
}

TEST_CASE("Random data transfers", "[trans4scif]") {
  auto s_pair = MakeConnectedNodes<std::shared_ptr<t4s::Socket>>(t4s::Listen, t4s::Connect);
  std::uniform_int_distribution<> dist(0, t4s::RECV_BUF_SIZE - t4s::CHUNK_HEAD_SIZE - 1);
  std::knuth_b eng(0);

  for (int i = 0; i < 100; ++i) {
    int sz = dist(eng);
    std::vector<uint8_t> sbuf(sz);
    std::for_each(sbuf.begin(), sbuf.end(), [&dist, &eng](uint8_t &n){ n = dist(eng) % 0xff; });
    std::vector<uint8_t> rbuf(sz);
    REQUIRE( sbuf.size() == s_pair[0]->Send(sbuf.data(), sbuf.size()) );
    REQUIRE( rbuf.size() == s_pair[1]->Recv(rbuf.data(), rbuf.size()) );
    REQUIRE( sbuf == rbuf );
  }
  //Verify if the recv buffer is
  std::vector<uint8_t> sbuf(t4s::RECV_BUF_SIZE - t4s::CHUNK_HEAD_SIZE);
  std::vector<uint8_t> rbuf(t4s::RECV_BUF_SIZE - t4s::CHUNK_HEAD_SIZE);
  REQUIRE( sbuf.size() == s_pair[0]->Send(sbuf.data(), sbuf.size()) );
  REQUIRE( rbuf.size() == s_pair[1]->Recv(rbuf.data(), rbuf.size()) );
}
