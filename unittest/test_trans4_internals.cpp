/*
	© Copyright 2015 CERN
	
	This software is distributed under the terms of the 
	GNU General Public Licence version 3 (GPL Version 3), 
	copied verbatim in the file “LICENSE”.
	In applying this licence, CERN does not waive 
	the privileges and immunities granted to it by virtue of its status 
	as an Intergovernmental Organization or submit itself to any jurisdiction.
	
	Author: Aram Santogidis <aram.santogidis@cern.ch>
*/

#include "catch.hpp"
#include <vector>
#include <cstdint>
#include <algorithm>
#include <chrono>
#include <thread>
#include "../src/scifnode.hpp"
#include "../src/util.hpp"
#include "../src/rmawindow.hpp"
#include "../src/ctl_messages.hpp"
#include "../src/virt_circbuf.hpp"
#include "../src/circbuf.hpp"
#include "../src/trans4node.hpp"


#ifdef XEONPHI
#define ADDR 0, 5555
#else
#define ADDR 5555
#endif

/**
 * Helper functions
 */
void barrier(ScifNode &sn)
{
	std::vector<uint8_t> msg(1);
	sn.sendMsg(msg);
	sn.recvMsg(1);
}

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

/**
 * ctl_messages module tests
 */
TEST_CASE("Test packing/unpacking of RMA_id ctl messages", "[ctl_messages]")
{
	std::vector<uint8_t> v = pack_RMA_id_msg({3,4});
	REQUIRE(sizeof(RMA_id) == v.size());
	RMA_id id = unpack_RMA_id_msg(v);
	REQUIRE(id.off == 3);
	REQUIRE(id.size == 4);
	REQUIRE(sizeof(RMA_id) == v.size());
}

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
/*
TEST_CASE("ScifNode RMAWindow_factory", "[scifnode]")
{
	ScifNode sn(ADDR);
	RMAWindow_factory win_factory = sn.create_RMAWindow_factory();

	SECTION("Generate window of one page")
	{
		RMAWindow win{win_factory.generate(0x1000, SCIF_PROT_WRITE)};
		REQUIRE( win.get_len() == 0x1000 );
		REQUIRE_FALSE( win.get_mem() == nullptr );
		REQUIRE_FALSE( win.get_off() == -1 );
	}

	SECTION("Generate window of non-multiple of page size")
	{
		RMAWindow win{win_factory.generate(0x565, SCIF_PROT_WRITE)};
		REQUIRE( win.get_len() == 0x1000 );
	}
	barrier(sn);
}*/

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

/**
 * circular buffer module tests
 */
 
TEST_CASE("Virt_circbuf tests", "[Virt_circbuf]")
{
	Virt_circbuf vcb(0X1000, 0X1000);

	SECTION("Wr and rd advance and space preservation")
	{
		REQUIRE( 0X40 == vcb.write(0x40) );
		REQUIRE( (0x1000-0x80) == vcb.get_space() );
		REQUIRE( 0X40 == vcb.read(0x40) );
		REQUIRE( (0x1000) == vcb.get_space() );
	}

	SECTION("Writing to full buffer")
	{
		vcb.write(0x1000);
		REQUIRE( 0 == vcb.get_space() );
		REQUIRE( 0 == vcb.write(1) );
	}

	SECTION("Reading from empty buffer")
	{
		REQUIRE( 0x1000 == vcb.get_space() );
		REQUIRE( 0 == vcb.read(1) );
		REQUIRE( 0x1000 == vcb.get_space() );
	}

	SECTION("check if wr wraps up when reaching to the end")
	{
		off_t wr = vcb.get_wr_rmaoff();
		vcb.write(0x1000);
		REQUIRE( wr == vcb.get_wr_rmaoff() );
	}

	SECTION("wr align")
	{
		vcb.write(0x33);
		REQUIRE( 0 == (vcb.get_wr_rmaoff() % CACHELINE_SIZE) );
	}

	SECTION("rd align")
	{
		REQUIRE( 0x63 == vcb.write(0x63) );
		REQUIRE( 0x1000-2*CACHELINE_SIZE == vcb.get_space() );
		REQUIRE( 0x20 == vcb.read(0x20) );
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

		REQUIRE( sizeof(payload) == cbuf.write(v.cbegin(), v.size()) );
		REQUIRE( (cbuf.get_wr_rmaoff() % 0x40) == 0 );

		cbuf.wr_update();
	}
	/** Test a case of a payload larger than the capacity of the buffer */
	barrier(sn);
}

/**
 * Trans4Node tests
 */

TEST_CASE("Trans4Node send/receive tests", "trans4node")
{
	Trans4Node tn(ADDR);

	SECTION("Simple message transmission")
	{
		std::vector<uint8_t> first_msg{'h', 'e', 'l', 'l', 'o'};

		REQUIRE( first_msg.size() == tn.send(first_msg) );
		std::this_thread::sleep_for(std::chrono::seconds(1));

		std::vector<uint8_t> recv_msg = tn.recv(first_msg.size());
		REQUIRE( recv_msg == first_msg );	
	}

	SECTION("Full buffer payload")
	{
		std::vector<uint8_t> buf_size_msg(RECV_BUF_SIZE - 8);
		REQUIRE( buf_size_msg.size() == tn.send(buf_size_msg) );
		std::this_thread::sleep_for(std::chrono::seconds(1));

		std::vector<uint8_t> recv_msg = tn.recv(buf_size_msg.size());
		REQUIRE( buf_size_msg.size() == recv_msg.size() );	
		std::this_thread::sleep_for(std::chrono::seconds(3));

		REQUIRE( buf_size_msg.size() == tn.send(buf_size_msg) );
		std::this_thread::sleep_for(std::chrono::seconds(1));

		recv_msg = tn.recv(buf_size_msg.size());
		REQUIRE( buf_size_msg.size() == recv_msg.size() );	
	}

	std::this_thread::sleep_for(std::chrono::seconds(1));
} 