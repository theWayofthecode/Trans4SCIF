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
#include <scifnode.h>
#include "catch.hpp"
#include "circbuf.h"
#include "test_common.h"

TEST_CASE("Virtt4s::Circbuf tests", "[virt_circbuf]")
{
  t4s::VirtCircbuf vcb(0X1000, 0X1000);

  SECTION("Wr and rd advance and space preservation")
  {
    REQUIRE( 0X40 == vcb.WrAdvance(0x40) );
    REQUIRE( (0x1000-0x40) == vcb.get_space() );
    REQUIRE( 0X40 == vcb.RdAdvance(0x40) );
    REQUIRE( (0x1000) == vcb.get_space() );
  }

  SECTION("Writing to full buffer")
  {
    vcb.WrAdvance(0x1000);
    REQUIRE( 0 == vcb.get_space() );
    REQUIRE( 0 == vcb.WrAdvance(1) );
  }

  SECTION("Reading from empty buffer")
  {
    REQUIRE( 0x1000 == vcb.get_space() );
    REQUIRE( 0 == vcb.RdAdvance(1) );
    REQUIRE( 0x1000 == vcb.get_space() );
  }

  SECTION("check if wr wraps up when reaching to the end")
  {
    off_t wr = vcb.get_wr_rmaoff();
    vcb.WrAdvance(0x1000);
    REQUIRE( wr == vcb.get_wr_rmaoff() );
  }

  SECTION("wr align")
  {
    vcb.WrAdvance(0x33);
    vcb.WrAlign();
    REQUIRE( 0 == (vcb.get_wr_rmaoff() % t4s::CACHELINE_SIZE) );
  }

  SECTION("rd align")
  {
    REQUIRE( 0x63 == vcb.WrAdvance(0x63) );
    vcb.WrAlign();
    REQUIRE( 0x1000-2*t4s::CACHELINE_SIZE == vcb.get_space() );
    REQUIRE( 0x20 == vcb.RdAdvance(0x20) );
    vcb.RdAlign();
    REQUIRE( (0x1000-t4s::CACHELINE_SIZE) == vcb.get_space() );
  }
}

TEST_CASE("circbuf tests", "[circbuf]")
{
  auto sn_pair = MakeConnectedNodes<t4s::ScifNode>();
  
  std::size_t circbuf_len = 0x1000;
  t4s::Circbuf cbuf(sn_pair[0]->CreateRMAWindow(circbuf_len, SCIF_PROT_WRITE));

  SECTION("Write read test")
  {
    REQUIRE( (cbuf.get_wr_rmaoff() % 0x40) == 0 );

    /* Prepare source */
    std::vector<uint8_t> v;
    int payload = 0xABCCBA;
    t4s::inttype_to_vec_le(payload, v);

    REQUIRE( sizeof(payload) == cbuf.Write(v.cbegin(), v.size()));
    cbuf.WrAlign();
    REQUIRE( (cbuf.get_wr_rmaoff() % 0x40) == 0 );

  }

  SECTION("RdReadResetChunkHead")
  {
    uint64_t head = 0xCB;
    std::vector<uint8_t> out;
    t4s::inttype_to_vec_le(head, out);
    cbuf.Write(out.cbegin(), out.size());

    REQUIRE( cbuf.RdReadResetChunkHead() == head );
  }

  SECTION("WrReadChunkHead")
  {
    off_t off_s = cbuf.get_wr_rmaoff();
    uint64_t head = 0xCB;
    std::vector<uint8_t> out;
    t4s::inttype_to_vec_le(head, out);
    cbuf.Write(out.cbegin(), out.size());
    cbuf.RdAdvance(sizeof(uint64_t));
    cbuf.WrAdvance(circbuf_len - sizeof(uint64_t));

    REQUIRE( 0 == cbuf.get_wr_rmaoff() - off_s );
    REQUIRE( cbuf.WrReadChunkHead() == head );
  }

  SECTION("WrResetChunkHead")
  {
    std::size_t circbuf_len = 0x1000;
    cbuf.WrResetChunkHead();
    cbuf.WrAlign();
    REQUIRE( 0 == cbuf.RdReadResetChunkHead() );
    cbuf.RdAlign();
  }
}
