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

#include <thread>
#include <chrono>
#include <system_error>
#include "catch.hpp"
#include "trans4scif.h"
#include "test_common.h"

std::array<std::unique_ptr<t4s::Socket>, 2> MakeConnectedSockets() {
  std::array<std::unique_ptr<t4s::Socket>, 2> s_pair;
  std::thread t_acc([&s_pair]() {
    s_pair[0].reset(t4s::CreateSocket(PORT));
  });
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  try {
    s_pair[1].reset(t4s::CreateSocket(0, PORT));
  } catch (std::system_error e) {
    std::cerr << "Warning: MakeConnectedNodes: " << e.what() << __FILE__LINE__ << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    s_pair[1].reset(t4s::CreateSocket(0, PORT));
  }
  t_acc.join();
  return s_pair;
}

TEST_CASE("Test version", "[trans4scif]") {
  REQUIRE(t4s::trans4scif_version() == "0.2");
}

TEST_CASE("Test Socket", "[trans4scif]") {
  auto s_pair = MakeConnectedSockets();

  SECTION("Simple message transmission") {
    std::vector<uint8_t> send_msg{'h', 'e', 'l', 'l', 'o'};
    REQUIRE(send_msg.size() == s_pair[0]->Send(send_msg.cbegin(), send_msg.size()));
    std::vector<uint8_t> recv_msg = s_pair[1]->Recv();
    REQUIRE(recv_msg == send_msg);

    std::vector<uint8_t> reply_msg{'h', 'i'};
    REQUIRE(reply_msg.size() == s_pair[1]->Send(reply_msg.cbegin(), reply_msg.size()));
    std::vector<uint8_t> recv_reply = s_pair[0]->Recv();
    REQUIRE(recv_reply == reply_msg);
  }
}