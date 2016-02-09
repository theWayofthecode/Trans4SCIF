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
#include "../src/util.hpp"
#include "../src/constants.hpp"

/**
 * util module tests
 */
TEST_CASE("Serializing/deserializing", "[util]")
{

	uint64_t out = 0, in = 0;
	std::vector<uint8_t> v;

	SECTION("Positive int")
	{
		in = 3;
		out = 0;
		inttype_to_vec_le(in, v);
		vec_to_inttype_le(v, out);
		REQUIRE(in == out);
	}

	SECTION("Zero")
	{
		in = 0;
		out = 1;
		inttype_to_vec_le(in, v);
		vec_to_inttype_le(v, out);
		REQUIRE(in == out);
	}

	SECTION("Negative int")
	{
		in = -1;
		out = 0;
		inttype_to_vec_le(in, v);
		vec_to_inttype_le(v, out);
		REQUIRE(in == out);
	}
}

TEST_CASE("ROUND_TO_BOUNDARY macro test", "[util]")
{
	REQUIRE( 0X40 == ROUND_TO_BOUNDARY(0x40, CACHELINE_SIZE) );
	REQUIRE( 0X80 == ROUND_TO_BOUNDARY(0x41, CACHELINE_SIZE) );
	std::size_t sz = 0x453;
	sz = ROUND_TO_BOUNDARY(sz, PAGE_SIZE);
	REQUIRE( 0X1000 == sz );
	sz = ROUND_TO_BOUNDARY(sz+1, PAGE_SIZE);
	REQUIRE( 0X2000 == sz );
	REQUIRE( 0X1000 == ROUND_TO_BOUNDARY(1, PAGE_SIZE) );
	REQUIRE( 0 == ROUND_TO_BOUNDARY(0, PAGE_SIZE) );
}