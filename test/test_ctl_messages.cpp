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
#include "trans4scif.hpp"
#include "ctl_messages.hpp"

TEST_CASE("Test packing/unpacking of RMA_id ctl messages", "[ctl_messages]")
{
    std::vector<uint8_t> v = pack_RMA_id_msg({3,4});
    REQUIRE(sizeof(RMA_id) == v.size());
    RMA_id id = unpack_RMA_id_msg(v);
    REQUIRE(id.off == 3);
    REQUIRE(id.size == 4);
    REQUIRE(sizeof(RMA_id) == v.size());
}