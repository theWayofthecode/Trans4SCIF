/*
	© Copyright 2015-2016 CERN
	
	This software is distributed under the terms of the 
	GNU General Public Licence version 3 (GPL Version 3), 
	copied verbatim in the file “LICENSE”.
	In applying this licence, CERN does not waive 
	the privileges and immunities granted to it by virtue of its status 
	as an Intergovernmental Organization or submit itself to any jurisdiction.
	
	Author: Aram Santogidis <aram.santogidis@cern.ch>
*/

#include "catch.hpp"
#include "common.hpp"
#include "../src/scifnode.hpp"
#include "../src/rmawindow.hpp"
#include "../src/virt_circbuf.hpp"
#include "../src/circbuf.hpp"

/**
 * circular buffer module tests
 */
 
TEST_CASE("Virt_circbuf tests", "[Virt_circbuf]")
{
	Virt_circbuf vcb(0X1000, 0X1000);

	SECTION("Wr and rd advance and space preservation")
	{
		REQUIRE( 0X40 == vcb.wr_advance(0x40) );
		REQUIRE( (0x1000-0x40) == vcb.get_space() );
		REQUIRE( 0X40 == vcb.rd_advance(0x40) );
		REQUIRE( (0x1000) == vcb.get_space() );
	}

	SECTION("Writing to full buffer")
	{
		vcb.wr_advance(0x1000);
		REQUIRE( 0 == vcb.get_space() );
		REQUIRE( 0 == vcb.wr_advance(1) );
	}

	SECTION("Reading from empty buffer")
	{
		REQUIRE( 0x1000 == vcb.get_space() );
		REQUIRE( 0 == vcb.rd_advance(1) );
		REQUIRE( 0x1000 == vcb.get_space() );
	}

	SECTION("check if wr wraps up when reaching to the end")
	{
		off_t wr = vcb.get_wr_rmaoff();
		vcb.wr_advance(0x1000);
		REQUIRE( wr == vcb.get_wr_rmaoff() );
	}

	SECTION("wr align")
	{
		vcb.wr_advance(0x33);
		vcb.wr_align();
		REQUIRE( 0 == (vcb.get_wr_rmaoff() % CACHELINE_SIZE) );
	}

	SECTION("rd align")
	{
		REQUIRE( 0x63 == vcb.wr_advance(0x63) );
		vcb.wr_align();
		REQUIRE( 0x1000-2*CACHELINE_SIZE == vcb.get_space() );
		REQUIRE( 0x20 == vcb.rd_advance(0x20) );
		vcb.rd_align();
		REQUIRE( (0x1000-CACHELINE_SIZE) == vcb.get_space() );
	}
}
 
TEST_CASE("circbuf tests", "[circbuf]")
{
	ScifNode sn(ADDR);
	SECTION("Write read test")
	{
		std::size_t circbuf_len = 0x1000;
		Circbuf cbuf(sn.create_RMAWindow(circbuf_len, SCIF_PROT_WRITE));

		REQUIRE( (cbuf.get_wr_rmaoff() % 0x40) == 0 );

		/* Prepare source */
		std::vector<uint8_t> v;
		int payload = 0xABCCBA;
		inttype_to_vec_le(payload, v);

		REQUIRE( sizeof(payload) == cbuf.write(v.cbegin(), v.size()));
		cbuf.wr_align();
		REQUIRE( (cbuf.get_wr_rmaoff() % 0x40) == 0 );

	}
	/** Test a case of a payload larger than the capacity of the buffer */
	barrier(sn);
}