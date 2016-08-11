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
#include "scifnode.h"
#include "util.h"
#include "catch.hpp"
#include "rmarecords.h"
#include "test_common.h"
#include "../src/trans4scif_config.h"

TEST_CASE("RMARecords read write scenarios", "[rmarecords]")
{
  auto sn_pair = MakeConnectedNodes<std::unique_ptr<t4s::ScifNode>>(
    [](int port) {return new t4s::ScifNode(port);}, //listener
    [](int node, int port) {return new t4s::ScifNode(node, port);} //connecter
  );

  t4s::RMAWindow win_send(sn_pair[0]->CreateRMAWindow(0x1000, SCIF_PROT_READ | SCIF_PROT_WRITE));
  t4s::RMAWindow win_recv(sn_pair[1]->CreateRMAWindow(0x1000, SCIF_PROT_READ | SCIF_PROT_WRITE));

  t4s::RMARecords sender(win_send.get_mem(), win_recv.get_mem(), win_recv.get_off());
  t4s::RMARecords recv(win_send.get_mem(), win_recv.get_mem(), -1);


  SECTION("Check read and write when recv_buf is empty")
  {
    REQUIRE_FALSE(recv.canRead());
    REQUIRE(sender.canWrite());
  }

  SECTION("Simple write read")
  {
    REQUIRE(sender.canWrite());
    off_t off = sender.written(100);
    REQUIRE(win_recv.get_off()+sizeof(uint64_t) == off);
    sn_pair[0]->SignalPeer(off, 100);
    t4s::Record rec = sender.getBufRec();
    REQUIRE(rec.start == 128);
    REQUIRE(rec.end == t4s::RECV_BUF_SIZE);
    rec = recv.getWrRec();
    REQUIRE(rec.start == 0);
    REQUIRE(rec.end == 100);
    REQUIRE(recv.canRead());
    recv.read(100);
    rec = sender.getBufRec();
    REQUIRE(rec.start == 128);
    REQUIRE(rec.end == t4s::RECV_BUF_SIZE);
    REQUIRE(win_recv.get_off()+sizeof(t4s::Record)+sizeof(uint64_t) == sender.written(t4s::RECV_BUF_SIZE-128));
    REQUIRE(sender.canWrite());
    rec = sender.getBufRec();
    REQUIRE(rec.start == 0);
    REQUIRE(rec.end == 128);
  }

  SECTION("Wr_records full case")
  {
    REQUIRE(sender.canWrite());
    off_t off = sender.written(10);
    REQUIRE(win_recv.get_off()+sizeof(uint64_t) == off);
    sn_pair[0]->SignalPeer(off, 10);
    for (int i = 0; i < 255; ++i) {
      REQUIRE(sender.canWrite());
      sender.written(10);
    }
    REQUIRE_FALSE(sender.canWrite());
    recv.read(10);
    REQUIRE(sender.canWrite());
  }
}

