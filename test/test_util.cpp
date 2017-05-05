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
#include "../src/trans4scif_config.h"

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

  SECTION("off_t with return value versions") {
    off_t off=0x1555;
    auto v = t4s::inttype_to_vec_le(off);
    REQUIRE(off == t4s::vec_to_inttype_le<off_t>(v));
  }
}

TEST_CASE("round template function test test", "[util]") {

  REQUIRE(0X40 == t4s::round<t4s::CL_SIZE>(0x40));
  REQUIRE(t4s::CL_SIZE == t4s::round<t4s::CL_SIZE>(1));
  REQUIRE(0X80 == t4s::round<t4s::CL_SIZE>(0x41));
  std::size_t sz = 0x453;
  sz = t4s::round<t4s::PAGE_SIZE>(sz);
  REQUIRE(0X1000 == sz);
  sz = t4s::round<t4s::PAGE_SIZE>(sz + 1);
  REQUIRE(0X2000 == sz);
  REQUIRE(0X1000 == t4s::round<t4s::PAGE_SIZE>(1));
  REQUIRE(0 == t4s::round<t4s::PAGE_SIZE>(0));

  SECTION("Round towards minus infiniti") {
    REQUIRE(0x40 == t4s::round<t4s::CL_SIZE>(0X40-(t4s::CL_SIZE-1)));
    REQUIRE(0x40 == t4s::round<t4s::CL_SIZE>(0X58-(t4s::CL_SIZE-1)));
  }
}
