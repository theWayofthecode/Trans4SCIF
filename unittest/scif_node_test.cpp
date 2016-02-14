/*
	C Copyright 2015-2016 CERN
	
	This software is distributed under the terms of the 
	GNU General Public Licence version 3 (GPL Version 3), 
	copied verbatim in the file "LICENSE".
	In applying this licence, CERN does not waive 
	the privileges and immunities granted to it by virtue of its status 
	as an Intergovernmental Organization or submit itself to any jurisdiction.
	
	Author: Aram Santogidis <aram.santogidis@cern.ch>
*/
#include <thread>
#include <chrono>
#include "catch.hpp"
#include "common.hpp"
#include "../src/ctl_messages.hpp"
#include "../src/scifnode.hpp"
#include "../src/rmawindow.hpp"

/**
 * scifnode module tests
 */
TEST_CASE("ScifNode send/receive", "[scifnode]")
{
	ScifNode sn(ADDR);


	SECTION("Empty message")
	{
		std::vector<uint8_t> vsend;
		REQUIRE( 0 == sn.sendMsg(vsend) );
		std::vector<uint8_t> vrecv = sn.recvMsg(0);
		REQUIRE( vrecv.empty() );
	}

	SECTION("Two ints")
	{
		int first = 0xaabb, second = 0xccdd, out = 0;

		std::vector<uint8_t> msg_send;
		inttype_to_vec_le(first, msg_send);
		inttype_to_vec_le(second, msg_send);
		REQUIRE ( 2*sizeof(first) == sn.sendMsg(msg_send) );

		std::vector<uint8_t> msg_recv = sn.recvMsg(2*sizeof(first));
		REQUIRE( 2*sizeof(first) == msg_recv.size() );
		vec_to_inttype_le(msg_recv, out);
		REQUIRE( first == out );
		msg_recv.erase(msg_recv.begin(), msg_recv.begin()+sizeof(first));
		vec_to_inttype_le(msg_recv, out);
		REQUIRE( second == out );
	}

	SECTION("Check messages in-order delivery")
	{
		int first = 0xaabb, second = 0xccdd, out = 0;

		std::vector<uint8_t> msg_first;
		inttype_to_vec_le(first, msg_first);
		REQUIRE ( sizeof(first) == sn.sendMsg(msg_first) );

		std::vector<uint8_t> msg_second;
		inttype_to_vec_le(second, msg_second);
		REQUIRE ( sizeof(second) == sn.sendMsg(msg_second) );

		std::vector<uint8_t> msg_recv1 = sn.recvMsg(sizeof(first));
		REQUIRE( sizeof(first) == msg_recv1.size() );
		vec_to_inttype_le(msg_recv1, out);
		REQUIRE( first == out );

		std::vector<uint8_t> msg_recv2 = sn.recvMsg(sizeof(second));
		REQUIRE( sizeof(second) == msg_recv2.size() );
		vec_to_inttype_le(msg_recv2, out);
		REQUIRE( second == out );
	}

	SECTION("Large message")
	{
		std::size_t sz = 0x1000-1; //Max recv buffer size
		std::vector<uint8_t> vsend(sz);
		REQUIRE( sz == sn.sendMsg(vsend) );
		std::vector<uint8_t> vrecv = sn.recvMsg(sz);
		REQUIRE( sz == vrecv.size() );
	}
	barrier(sn);
}

TEST_CASE("ScifNode writeMsg", "[scifnode]")
{
	ScifNode sn(ADDR);

	SECTION("Write receive test", "[scifnode]")
	{ //The scope is necessary to make sure that the RMAWindows are destroyed before ScifNodes are destroyed
		RMAWindow win_recv(sn.create_RMAWindow(0x1000, SCIF_PROT_WRITE));
		RMAWindow win_send(sn.create_RMAWindow(0x1000, SCIF_PROT_READ));

		/* Exchange RMA offs with peer */
		std::vector<uint8_t> msg = pack_RMA_id_msg({win_recv.get_off(), win_recv.get_len()});
		REQUIRE( sizeof(RMA_id) == msg.size() );
		REQUIRE( sizeof(RMA_id) == sn.sendMsg(msg) );
		
		msg = sn.recvMsg(sizeof(RMA_id));
		REQUIRE( sizeof(RMA_id) == msg.size() );
		RMA_id rem_id = unpack_RMA_id_msg(msg);

		/* Prepare source */
		std::vector<uint8_t> v;
		int payload = 0xABCCBA;

		inttype_to_vec_le(payload, v);
		std::copy(v.begin(), v.end(), static_cast<uint8_t *>(win_send.get_mem()));

		/* RDMA write */
		sn.writeMsg(rem_id.off, win_send.get_off(), sizeof(payload));
		barrier(sn);
		
		/* Read receive buffer */
		uint8_t * src = static_cast<uint8_t *>(win_recv.get_mem());
		std::copy(src, src+sizeof(payload), v.begin());
		int result = 0;
		vec_to_inttype_le(v, result);

		/* Verify the result */
		REQUIRE( payload == result );
	}
	barrier(sn);
}

TEST_CASE("ScifNode signalPeer", "[scifnode]")
{
	ScifNode sn(ADDR);

	SECTION("Peer node notification test")
	{
		RMAWindow win{sn.create_RMAWindow(0x1000, SCIF_PROT_WRITE)};

		/* Exchange RMA offs with peer */
		std::vector<uint8_t> msg = pack_RMA_id_msg({win.get_off(), win.get_len()});
		sn.sendMsg(msg);
		
		msg = sn.recvMsg(sizeof(RMA_id));
		RMA_id rem_id = unpack_RMA_id_msg(msg);

		sn.signalPeer(rem_id.off, 0xABABA);
		while (*(static_cast<uint64_t *>(win.get_mem())) != 0xABABA);
	}
	barrier(sn);
}

TEST_CASE("ScifNode has_recv_msg", "[scifnode]")
{
	ScifNode sn(ADDR);

	std::vector<uint8_t> msg(16);
	REQUIRE( sn.sendMsg(msg) == 16 );
	barrier(sn);
	REQUIRE( sn.has_recv_msg() );
	std::vector<uint8_t> v = sn.recvMsg(16);
	REQUIRE( v.size() == 16 );
	REQUIRE_FALSE( sn.has_recv_msg() ); 

	// TODO: find out why the barrier doesn't work
	std::this_thread::sleep_for(std::chrono::seconds(1));
	barrier(sn);
}
