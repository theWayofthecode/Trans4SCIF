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

#include "../src/trans4node.hpp"

/**
 * Trans4Node tests
 */

TEST_CASE("Trans4Node send/receive tests", "trans4node")
{
	//We need an additiona scifnode for barriers (what if the listener is slower than the connector?)
	#ifdef XEONPHI
		std::this_thread::sleep_for(std::chrono::seconds(1));
		ScifNode sn(0, 6666);
		std::this_thread::sleep_for(std::chrono::seconds(1));
	#else
		ScifNode sn(6666);
	#endif
	Trans4Node tn(ADDR);

	SECTION("Simple message transmission")
	{
		std::vector<uint8_t> first_msg{'h', 'e', 'l', 'l', 'o'};
		REQUIRE( first_msg.size() == tn.send(first_msg.cbegin(), first_msg.size()) );
		std::this_thread::sleep_for(std::chrono::seconds(3));

		std::vector<uint8_t> recv_msg(first_msg.size());
		recv_msg[0]='x';
		REQUIRE( first_msg.size() == tn.recv(recv_msg.begin(), recv_msg.size()) );
		REQUIRE( recv_msg == first_msg );
		WARN ( recv_msg[0] );
	}
/*
	SECTION("Full buffer payload")
	{
		std::vector<uint8_t> buf_size_msg(RECV_BUF_SIZE - 8);
		REQUIRE( buf_size_msg.size() == tn.send(buf_size_msg.cbegin(), buf_size_msg.size()) );
		std::this_thread::sleep_for(std::chrono::seconds(3));
		std::vector<uint8_t> recv_msg(RECV_BUF_SIZE - 8);	
		REQUIRE( buf_size_msg.size() == tn.recv(recv_msg.begin(), recv_msg.size()) );	
	}*/
		std::this_thread::sleep_for(std::chrono::seconds(1));
	barrier(sn);
		std::this_thread::sleep_for(std::chrono::seconds(1));
} 