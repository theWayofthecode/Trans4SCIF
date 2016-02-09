#include "../src/trans4node.hpp"

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