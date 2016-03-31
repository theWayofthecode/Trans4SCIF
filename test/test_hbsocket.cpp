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

#include <memory>
#include <iostream>
#include <scifnode.h>
#include "catch.hpp"
#include "hbsocket.h"
#include "test_common.h"

//TODO: std::size_t Recv test

TEST_CASE("HBSocket send/receive tests", "[hbsocket]")
{
  auto s_pair = MakeConnectedNodes<std::unique_ptr<t4s::HBSocket>>(
      [](int port) {return new t4s::HBSocket(port);}, //listener
      [](int node, int port) {return new t4s::HBSocket(node, port);} //connecter
  );

  SECTION("Simple message transmission")
  {
    std::vector<uint8_t> send_msg{'h', 'e', 'l', 'l', 'o'};
    REQUIRE( send_msg.size() == s_pair[0]->Send(send_msg.cbegin(), send_msg.size()) );
    std::vector<uint8_t> recv_msg = s_pair[1]->Recv();
    REQUIRE( recv_msg == send_msg );
  }

  SECTION("Simple Full buffer transfer test")
  {
    std::vector<uint8_t> buf_size_msg(t4s::RECV_BUF_SIZE - 8);
    for (uint8_t &vi : buf_size_msg) {
      vi = 0x40;
    }
    REQUIRE( buf_size_msg.size() == s_pair[0]->Send(buf_size_msg.cbegin(), buf_size_msg.size()) );
    REQUIRE( buf_size_msg == s_pair[1]->Recv() );
  }

  SECTION("Buffer wrap up case")
  {
    std::vector<uint8_t> buf_size_msg(t4s::RECV_BUF_SIZE - 8);
    for (uint8_t &vi : buf_size_msg) {
      vi = 0x40;
    }
    REQUIRE( buf_size_msg.size()/2 == s_pair[0]->Send(buf_size_msg.cbegin(), buf_size_msg.size()/2) );
    REQUIRE( buf_size_msg.size()/2 == s_pair[1]->Recv().size() );
    REQUIRE( buf_size_msg.size() == s_pair[0]->Send(buf_size_msg.cbegin(), buf_size_msg.size()) );
    REQUIRE( buf_size_msg.size() == s_pair[1]->Recv().size() );
  }
}
