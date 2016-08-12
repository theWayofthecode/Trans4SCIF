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

  for (int i = 0; i < 10; ++i) {
    int sz = dist(eng);
    std::vector<uint8_t> sbuf(sz);
    std::for_each(sbuf.begin(), sbuf.end(), [&dist, &eng](uint8_t &n){ n = dist(eng) % 0xff; });
    std::vector<uint8_t> rbuf(sz);
    INFO("i: " << i << " sz: " << sz);
    REQUIRE( sbuf.size() == s_pair[0]->Send(sbuf.data(), sbuf.size()) );
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    REQUIRE( rbuf.size() == s_pair[1]->Recv(rbuf.data(), rbuf.size()) );
    REQUIRE( sbuf == rbuf );
  }
}
