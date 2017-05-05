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
#include <random>
#include <future>
#include <thread>
#include <array>
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

//////////////////// TESTS //////////////////////////

TEST_CASE("Test version", "[trans4scif]") {
  std::cerr << t4s::trans4scif_config();
}
//
//TEST_CASE("t4s Socket initialization", "[trans4scif]") {
//  //initialize the randomizer
//  std::uniform_int_distribution<> dist(0, t4s::SCIF_SEND_RECV_TIMOUT - 100);
//  std::knuth_b eng(0);
//
//  for (int i = 0; i < 100; ++i) {
//    auto fut = std::async(std::launch::async, [&dist, &eng]() -> std::unique_ptr<t4s::Socket> {
//      std::this_thread::sleep_for(std::chrono::milliseconds(dist(eng)));
//      return std::unique_ptr<t4s::Socket>(t4s::listeningSocket(PORT));
//    });
//    //The for-loop is used in order to handle connect-retries in case of ECONNREFUSED
//    for (;;) {
//      try {
//        std::this_thread::sleep_for(std::chrono::milliseconds(dist(eng)));
//        std::unique_ptr<t4s::Socket> consock(t4s::connectingSocket(0, PORT));
//        std::unique_ptr<t4s::Socket> accsock(fut.get());
//        uint8_t s = 'x', r = 0;
//
//        //To make sure that the pair is connected and we can send
//        REQUIRE(1 == consock->send(&s, 1));
//        REQUIRE(1 == accsock->recv(&r, 1));
//        REQUIRE(r == 'x');
//        REQUIRE(1 == accsock->send(&s, 1));
//        REQUIRE(1 == consock->recv(&r, 1));
//        REQUIRE(r == 'x');
//        break;
//      } catch (std::system_error se) {
//        if (se.code().value() != ECONNREFUSED)
//          throw;
//      }
//    }
//  }
//}

TEST_CASE("send one byte", "[trans4scif]") {
  auto s_pair = MakeConnectedNodes<t4s::Socket>();
  uint8_t s = 'x';
  uint8_t r = 0;
  REQUIRE(1 == s_pair[0]->send(&s, 1));
  REQUIRE(1 == s_pair[1]->recv(&r, 1));
  REQUIRE(r == 'x');
}

TEST_CASE("Send and recv 0 bytes", "[trans4scif]") {
  auto s_pair = MakeConnectedNodes<t4s::Socket>();
  REQUIRE(0 == s_pair[0]->send(nullptr, 0));
  REQUIRE(0 == s_pair[1]->recv(nullptr, 0));
}

TEST_CASE("Random data transfers", "[trans4scif]") {
  auto s_pair = MakeConnectedNodes<t4s::Socket>();
  // Receiver
  std::thread trecv ([&s_pair]() {
    std::uniform_int_distribution<> dist(0, t4s::BUF_SIZE - t4s::CHUNK_HEAD_SIZE - 1);
    std::knuth_b eng(0);
    std::unique_ptr<uint8_t[]> rbuf(new uint8_t[t4s::BUF_SIZE]);

    for (int i = 0; i < 100; ++i) {
      int sz = dist(eng);
      for (int i = 0;
           i < sz;
           i += s_pair[1]->recv(rbuf.get(), std::min(sz - i, static_cast<int>(t4s::BUF_SIZE)))
          );
    }
  });

  // Sender
  std::uniform_int_distribution<> dist(0, t4s::BUF_SIZE - t4s::CHUNK_HEAD_SIZE - 1);
  std::knuth_b eng(0);
  std::unique_ptr<uint8_t[]> sbuf(new uint8_t[t4s::BUF_SIZE]);
  int i;
  for (i = 0; i < 100; ++i) {
    int sz = dist(eng);
    for (int i = 0;
         i < sz;
         i += s_pair[0]->send(sbuf.get(), std::min(sz - i, static_cast<int>(t4s::BUF_SIZE)))
        );
  }
  REQUIRE(i == 100);
  trecv.join();
}


TEST_CASE("Exponential increasing size", "[trans4scif]") {
  auto s_pair = MakeConnectedNodes<t4s::Socket>();

  // Receiver
  std::thread trecv ([&s_pair]() {
    std::unique_ptr<uint8_t[]> rbuf(new uint8_t[t4s::BUF_SIZE]);
    for (int sz = 64; sz < (1 << 29); sz <<= 1) {
      for (int j = 0; j < 10; ++j) {
        for (int i = 0;
             i < sz;
             i += s_pair[1]->recv(rbuf.get(), std::min(sz - i, static_cast<int>(t4s::BUF_SIZE)))
            );
      }
    }
  });

  // Sender
  std::unique_ptr<uint8_t[]> sbuf(new uint8_t[t4s::BUF_SIZE]);
  for (int sz = 64; sz < (1 << 29); sz <<= 1) {
    for (int j = 0; j < 10; ++j) {
      for (int i = 0;
           i < sz;
           i += s_pair[0]->send(sbuf.get(), std::min(sz - i, static_cast<int>(t4s::BUF_SIZE)))
          );
    }
  }
  trecv.join();
}

//TEST_CASE("Socket from connected epd test", "[trans4scif]") {
//auto s_pair = MakeConnectedNodes<t4s::Socket>();
//  uint8_t s = 'x';
//  uint8_t r = 0;
//
//  auto sn0_fut = std::async(std::launch::async, [&s_pair]() -> t4s::Socket * {
//      return new t4s::Socket(s_pair[0]);
//    });
//  std::unique_ptr<t4s::Socket> s1(new t4s::Socket(s_pair[1]));
//  std::unique_ptr<t4s::Socket> s0(sn0_fut.get());
//
//  REQUIRE(1 == s0->send(&s, 1));
//  REQUIRE(1 == s1->recv(&r, 1));
//  REQUIRE(r == 'x');
//}

TEST_CASE("trans4scif->recv blocking", "[trans4scif]") {
  auto s_pair = MakeConnectedNodes<t4s::Socket>();
  uint8_t r = 0;

  SECTION("Blocking for 1 second") {
    std::async(std::launch::async, [&s_pair]() {
      uint8_t s = 'x';
      std::this_thread::sleep_for(std::chrono::seconds(1));
      REQUIRE(1 == s_pair[0]->send(&s, 1));
    });
    s_pair[1]->waitIn(3000);
    REQUIRE(1 == s_pair[1]->recv(&r, 1));
    REQUIRE(r == 'x');
  }
}