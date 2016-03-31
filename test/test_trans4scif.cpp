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
#include <future>
#include <scif.h>
#include "catch.hpp"
#include "trans4scif.h"
#include "test_common.h"
#include "scifepd.h"

TEST_CASE("Test version", "[trans4scif]") {
  REQUIRE(t4s::trans4scif_version() == "0.2");
}

void hello_hi (std::shared_ptr<t4s::Socket> s0, std::shared_ptr<t4s::Socket> s1) {
  std::vector<uint8_t> send_msg{'h', 'e', 'l', 'l', 'o'};
  REQUIRE(send_msg.size() == s0->Send(send_msg.cbegin(), send_msg.size()));
  std::vector<uint8_t> recv_msg = s1->Recv();
  REQUIRE(recv_msg == send_msg);

  std::vector<uint8_t> reply_msg{'h', 'i'};
  REQUIRE(reply_msg.size() == s1->Send(reply_msg.cbegin(), reply_msg.size()));
  std::vector<uint8_t> recv_reply = s0->Recv();
  REQUIRE(recv_reply == reply_msg);
}

TEST_CASE("Test Socket", "[trans4scif]") {
  auto s_pair = MakeConnectedNodes<std::shared_ptr<t4s::Socket>>(t4s::Listen, t4s::Connect);
  hello_hi (s_pair[0], s_pair[1]);
}

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

TEST_CASE("Test SocketFromEpd", "[trans4scif]") {
  auto epd_pair = MakeConnectedNodes<scif_epd_t>(plain_scif_listen, plain_scif_connect);

/*  SECTION("Synchronous") {
    auto s1_fut = std::async(std::launch::async, t4s::SocketFromEpd, epd_pair[1]);
    std::shared_ptr<t4s::Socket> s0(t4s::SocketFromEpd(epd_pair[0]));
    std::shared_ptr<t4s::Socket> s1(s1_fut.get());
    hello_hi(s0, s1);
  }*/

  SECTION("Asynchronous") {
    auto s0_fut = t4s::SocketFromEpdAsync(epd_pair[0]);
    auto s1_fut = t4s::SocketFromEpdAsync(epd_pair[1]);
    std::shared_ptr<t4s::Socket> s0(s0_fut.get());
    std::shared_ptr<t4s::Socket> s1(s1_fut.get());
    hello_hi(s0, s1);
  }
}