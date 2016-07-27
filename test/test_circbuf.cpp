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
#include "../src/trans4scif_config.h"

TEST_CASE("VirtCircbuf basic tests", "[virt_circbuf]")
{
  t4s::VirtCircbuf vcb(0, t4s::RECV_BUF_SIZE);

  SECTION("Wr and rd advance and space preservation")
  {
    REQUIRE( 0X40 == vcb.WrAdvance(t4s::CACHELINE_SIZE) );
    REQUIRE( (t4s::RECV_BUF_SIZE-t4s::CACHELINE_SIZE) == vcb.get_space() );
    REQUIRE( 0X40 == vcb.RdAdvance(t4s::CACHELINE_SIZE) );
    REQUIRE( (t4s::RECV_BUF_SIZE) == vcb.get_space() );
  }

  SECTION("Writing to full buffer")
  {
    vcb.WrAdvance(t4s::RECV_BUF_SIZE);
    REQUIRE( 0 == vcb.get_space() );
    REQUIRE( 0 == vcb.WrAdvance(1) );
  }

  SECTION("Reading from empty buffer")
  {
    REQUIRE( t4s::RECV_BUF_SIZE == vcb.get_space() );
    REQUIRE( 0 == vcb.RdAdvance(1) );
    REQUIRE( t4s::RECV_BUF_SIZE == vcb.get_space() );
  }

  SECTION("check if wr wraps up when reaching at the end")
  {
    off_t wr = vcb.get_wr_rmaoff();
    vcb.WrAdvance(t4s::RECV_BUF_SIZE);
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
    REQUIRE( t4s::RECV_BUF_SIZE-2*t4s::CACHELINE_SIZE == vcb.get_space() );
    REQUIRE( 0x20 == vcb.RdAdvance(0x20) );
    vcb.RdAlign();
    REQUIRE( (t4s::RECV_BUF_SIZE-t4s::CACHELINE_SIZE) == vcb.get_space() );
  }
}

TEST_CASE("circbuf tests", "[circbuf]")
{
  auto sn_pair = MakeConnectedNodes<std::unique_ptr<t4s::ScifNode>>(
      [](int port) {return new t4s::ScifNode(port);}, //listener
      [](int node, int port) {return new t4s::ScifNode(node, port);} //connecter
  );
  
  std::size_t circbuf_len = t4s::RECV_BUF_SIZE;
  t4s::Circbuf cbuf(sn_pair[0]->CreateRMAWindow(circbuf_len, SCIF_PROT_WRITE));

  SECTION("Write Read test")
  {
    REQUIRE( (cbuf.get_wr_rmaoff() % t4s::CACHELINE_SIZE) == 0 );

    /* Prepare source */
    std::vector<uint8_t> v;
    int payload = 0xABCCBA;
    t4s::inttype_to_vec_le(payload, v);

    REQUIRE( sizeof(payload) == cbuf.Write(v.data(), v.size()));
    cbuf.WrAlign();
    REQUIRE( (cbuf.get_wr_rmaoff() % t4s::CACHELINE_SIZE) == 0 );

    std::vector<uint8_t> x(v.size());
    int out = 0;
    REQUIRE( cbuf.Read(x.data(), x.size()) == x.size() );
    t4s::vec_to_inttype_le(x, out);
    REQUIRE( out == payload);

  }

  SECTION("RdReadResetChunkHead")
  {
    uint64_t head = 0xCB;
    std::vector<uint8_t> out;
    t4s::inttype_to_vec_le(head, out);
    cbuf.Write(out.data(), out.size());

    REQUIRE( cbuf.RdReadResetChunkHead() == head );
  }

  SECTION("WrReadChunkHead")
  {
    off_t off_s = cbuf.get_wr_rmaoff();
    uint64_t head = 0xCB;
    std::vector<uint8_t> out;
    t4s::inttype_to_vec_le(head, out);
    cbuf.Write(out.data(), out.size());
    cbuf.RdAdvance(sizeof(uint64_t));
    cbuf.WrAdvance(circbuf_len - sizeof(uint64_t));

    REQUIRE( 0 == cbuf.get_wr_rmaoff() - off_s );
    REQUIRE( cbuf.WrReadChunkHead() == head );
  }

  SECTION("WrResetChunkHead")
  {
    std::size_t circbuf_len = t4s::RECV_BUF_SIZE;
    cbuf.WrResetChunkHead();
    cbuf.WrAlign();
    REQUIRE( 0 == cbuf.RdReadResetChunkHead() );
    cbuf.RdAlign();
  }

  SECTION("cirbuf wrap-around check")
  {
    std::vector<uint8_t> first(t4s::RECV_BUF_SIZE - 100);
    std::vector<uint8_t> v(128);
    //first
    REQUIRE( cbuf.Write(first.data(), first.size()) == first.size() );
    cbuf.WrAlign();
    REQUIRE( cbuf.Read(first.data(), first.size()) == first.size() );
    cbuf.RdAlign();
    //wrap around
    //Write does not wrap around automatically, but Read does
    auto w1 = cbuf.Write(v.data(), v.size());
    REQUIRE( w1 < v.size() );  
    auto w2 = cbuf.Write(v.data()+w1, v.size()-w1);
    REQUIRE( w2+w1 == v.size() );
    cbuf.WrAlign();

    REQUIRE( cbuf.Read(v.data(), v.size()) == v.size() );
    cbuf.RdAlign();
  }

  SECTION("Test sz > MAX_BUF_SIZE")
  {
    std::vector<uint8_t> wbuf(t4s::RECV_BUF_SIZE+1);
    std::fill_n(wbuf.begin(), wbuf.size(), 0xAB);
    std::vector<uint8_t> rbuf(t4s::RECV_BUF_SIZE+1);
    REQUIRE( t4s::RECV_BUF_SIZE == cbuf.Write(wbuf.data(), wbuf.size()) );
    REQUIRE( t4s::RECV_BUF_SIZE == cbuf.Read(rbuf.data(), rbuf.size()) );
    REQUIRE( std::all_of(rbuf.begin(), rbuf.end()-1, [](uint8_t a) {return a == 0xAB;}) );
  }
}
