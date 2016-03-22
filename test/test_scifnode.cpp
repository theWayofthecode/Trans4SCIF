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
#include "catch.hpp"
#include "scifnode.h"
#include "ctl_messages.h"
#include "test_common.h"

TEST_CASE("ScifNode send/receive", "[scifnode]")
{
  auto sn_pair = MakeConnectedNodes<t4s::ScifNode>();

  SECTION("Empty message")
  {
    std::vector<uint8_t> vsend;
    REQUIRE( 0 == sn_pair[0]->SendMsg(vsend) );
    std::vector<uint8_t> vrecv = sn_pair[1]->RecvMsg(0);
    REQUIRE( vrecv.empty() );
  }

  SECTION("Two ints")
  {
    int first = 0xaabb, second = 0xccdd, out = 0;

    std::vector<uint8_t> msg_send;
    t4s::inttype_to_vec_le(first, msg_send);
    t4s::inttype_to_vec_le(second, msg_send);
    REQUIRE ( 2*sizeof(first) == sn_pair[0]->SendMsg(msg_send) );

    std::vector<uint8_t> msg_recv = sn_pair[1]->RecvMsg(2*sizeof(first));
    REQUIRE( 2*sizeof(first) == msg_recv.size() );
    t4s::vec_to_inttype_le(msg_recv, out);
    REQUIRE( first == out );
    msg_recv.erase(msg_recv.begin(), msg_recv.begin()+sizeof(first));
    t4s::vec_to_inttype_le(msg_recv, out);
    REQUIRE( second == out );
  }

  SECTION("Check messages in-order delivery")
  {
    int first = 0xaabb, second = 0xccdd, out = 0;

    std::vector<uint8_t> msg_first;
    t4s::inttype_to_vec_le(first, msg_first);
    REQUIRE ( sizeof(first) == sn_pair[0]->SendMsg(msg_first) );

    std::vector<uint8_t> msg_second;
    t4s::inttype_to_vec_le(second, msg_second);
    REQUIRE ( sizeof(second) == sn_pair[0]->SendMsg(msg_second) );

    std::vector<uint8_t> msg_recv1 = sn_pair[1]->RecvMsg(sizeof(first));
    REQUIRE( sizeof(first) == msg_recv1.size() );
    t4s::vec_to_inttype_le(msg_recv1, out);
    REQUIRE( first == out );

    std::vector<uint8_t> msg_recv2 = sn_pair[1]->RecvMsg(sizeof(second));
    REQUIRE( sizeof(second) == msg_recv2.size() );
    t4s::vec_to_inttype_le(msg_recv2, out);
    REQUIRE( second == out );
  }

  SECTION("Large message")
  {
    std::size_t sz = 0x1000-1; //Max recv buffer size
    std::vector<uint8_t> vsend(sz);
    REQUIRE( sz == sn_pair[0]->SendMsg(vsend) );
    std::vector<uint8_t> vrecv = sn_pair[1]->RecvMsg(sz);
    REQUIRE( sz == vrecv.size() );
  }
}

TEST_CASE("ScifNode writeMsg", "[scifnode]")
{
  auto sn_pair = MakeConnectedNodes<t4s::ScifNode>();

  SECTION("Write receive test", "[scifnode]")
  {
    t4s::RMAWindow win_send(sn_pair[0]->CreateRMAWindow(0x1000, SCIF_PROT_READ));
    t4s::RMAWindow win_recv(sn_pair[1]->CreateRMAWindow(0x1000, SCIF_PROT_WRITE));

//     Prepare source
    std::vector<uint8_t> v;
    int payload = 0xABCCBA;

    t4s::inttype_to_vec_le(payload, v);
    std::copy(v.begin(), v.end(), static_cast<uint8_t *>(win_send.get_mem()));

//     RDMA write
    sn_pair[0]->WriteMsg(win_recv.get_off(), win_send.get_off(), sizeof(payload));

//     Read receive buffer
    uint8_t * src = static_cast<uint8_t *>(win_recv.get_mem());
    std::copy(src, src+sizeof(payload), v.begin());
    int result = 0;
    t4s::vec_to_inttype_le(v, result);

//     Verify the result
    REQUIRE( payload == result );
  }
}

TEST_CASE("t4s::ScifNode has_recv_msg", "[scifnode]")
{
  auto sn_pair = MakeConnectedNodes<t4s::ScifNode>();

  std::vector<uint8_t> msg(16);
  REQUIRE( sn_pair[0]->SendMsg(msg) == 16 );
  REQUIRE( sn_pair[1]->HasRecvMsg() );
  std::vector<uint8_t> v = sn_pair[1]->RecvMsg(16);
  REQUIRE( v.size() == 16 );
  REQUIRE_FALSE( sn_pair[1]->HasRecvMsg() );
}
