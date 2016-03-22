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
#include "hbsocket.h"
#include "ctl_messages.h"

TEST_CASE("Test packing/unpacking of RMAId ctl messages", "[ctl_messages]") {
  std::vector<uint8_t> v = t4s::PackRMAIdMsg({3, 4});
  REQUIRE(sizeof(t4s::RMAId) == v.size());
  t4s::RMAId id = t4s::UnpackRMAIdMsg(v);
  REQUIRE(id.off == 3);
  REQUIRE(id.size == 4);
  REQUIRE(sizeof(t4s::RMAId) == v.size());
}