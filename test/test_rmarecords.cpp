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
#include "trans4scif.h"
#include "rmarecordsreader.h"
#include "rmarecordswriter.h"
#include "test_common.h"
#include "../src/trans4scif_config.h"
#include "../include/trans4scif.h"

TEST_CASE("RMARecords read write scenarios", "[rmarecords]")
{
  auto sn_pair = MakeConnectedNodes<t4s::ScifNode>();

  t4s::RMAWindow win_send(sn_pair[0]->createRMAWindow(0x1000, SCIF_PROT_READ | SCIF_PROT_WRITE));
  t4s::RMAWindow win_recv(sn_pair[1]->createRMAWindow(0x1000, SCIF_PROT_READ | SCIF_PROT_WRITE));
  t4s::Record inval;
  inval.start = inval.end = -1;
  std::fill_n(static_cast<t4s::Record *>(win_recv.mem()), win_recv.size()/sizeof(t4s::Record), inval);
  auto mmap_wr = sn_pair[0]->createMmapmem(win_recv.off(), 0x1000, SCIF_PROT_READ | SCIF_PROT_WRITE);
  auto mmap_buf = sn_pair[1]->createMmapmem(win_send.off(), 0x1000, SCIF_PROT_READ | SCIF_PROT_WRITE);
  REQUIRE(win_send.off() == mmap_buf.off());
  REQUIRE(win_recv.off() == mmap_wr.off());
  off_t win_recv_off = win_recv.off();
  off_t win_send_off = win_send.off();
  t4s::RMARecordsWriter sender(win_send, mmap_wr, t4s::BUF_SIZE);
  t4s::RMARecordsReader recv(mmap_buf, win_recv, t4s::BUF_SIZE);

  SECTION("Check read and write when recv_buf is empty")
  {
    REQUIRE_FALSE(recv.canRead());
    REQUIRE(sender.canWrite());
  }

  SECTION("Simple write read")
  {
    REQUIRE(sender.canWrite());
    off_t off = sender.written(100);
    REQUIRE(win_recv_off+sizeof(uint64_t) == off);
    sn_pair[0]->signalPeer(off, 100);
    t4s::Record rec = sender.getBufRec();
    REQUIRE(rec.start == 128);
    REQUIRE(rec.end == t4s::BUF_SIZE);
    rec = recv.getWrRec();
    REQUIRE(rec.start == 0);
    REQUIRE(rec.end == 100);
    REQUIRE(recv.canRead());
    recv.read(100);
    rec = sender.getBufRec();
    REQUIRE(rec.start == 128);
    REQUIRE(rec.end == t4s::BUF_SIZE);
    REQUIRE(win_recv_off+sizeof(t4s::Record)+sizeof(uint64_t) == sender.written(t4s::BUF_SIZE-128));
    REQUIRE(sender.canWrite());
    rec = sender.getBufRec();
    REQUIRE(rec.start == 0);
    REQUIRE(rec.end == 128);
  }

  SECTION("Wr_records full case")
  {
    REQUIRE(sender.canWrite());
    off_t off = sender.written(10);
    REQUIRE(win_recv_off+sizeof(uint64_t) == off);
    sn_pair[0]->signalPeer(off, 10);
    for (int i = 0; i < 255; ++i) {
      REQUIRE(sender.canWrite());
      sender.written(10);
    }
    REQUIRE_FALSE(sender.canWrite());
    recv.read(10);
    REQUIRE(sender.canWrite());
  }
}

