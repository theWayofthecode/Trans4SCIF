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

#include "catch.hpp"
#include "util.h"
#include "constants.h"

TEST_CASE("Serializing/deserializing", "[util]") {

  uint64_t out = 0, in = 0;
  std::vector<uint8_t> v;

  SECTION("Positive int") {
    in = 3;
    out = 0;
    t4s::inttype_to_vec_le(in, v);
    t4s::vec_to_inttype_le(v, out);
    REQUIRE(in == out);
  }

  SECTION("Zero") {
    in = 0;
    out = 1;
    t4s::inttype_to_vec_le(in, v);
    t4s::vec_to_inttype_le(v, out);
    REQUIRE(in == out);
  }

  SECTION("Negative int") {
    in = -1;
    out = 0;
    t4s::inttype_to_vec_le(in, v);
    t4s::vec_to_inttype_le(v, out);
    REQUIRE(in == out);
  }
}

TEST_CASE("ROUND_TO_BOUNDARY macro test", "[util]") {

  REQUIRE(0X40 == ROUND_TO_BOUNDARY(0x40, t4s::CACHELINE_SIZE));
  REQUIRE(0X80 == ROUND_TO_BOUNDARY(0x41, t4s::CACHELINE_SIZE));
  std::size_t sz = 0x453;
  sz = ROUND_TO_BOUNDARY(sz, t4s::PAGE_SIZE);
  REQUIRE(0X1000 == sz);
  sz = ROUND_TO_BOUNDARY(sz + 1, t4s::PAGE_SIZE);
  REQUIRE(0X2000 == sz);
  REQUIRE(0X1000 == ROUND_TO_BOUNDARY(1, t4s::PAGE_SIZE));
  REQUIRE(0 == ROUND_TO_BOUNDARY(0, t4s::PAGE_SIZE));
}