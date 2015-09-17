#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include "scifnode.hpp"
#include "rmawindowref.hpp"

void connecter (uint16_t target_node_id, uint16_t target_port)
{
	ScifNode n(target_node_id, target_port);
	std::vector<uint8_t> msg(4);
	n.recvMsg(msg);
	for (auto i: msg)
		std::cout << std::hex << i;
	std::cout << std::endl;
}

void listener (uint16_t listening_port)
{
	ScifNode n(listening_port);
	{
	auto winref = n.openRMAWindow(1);
	std::cout << winref.off << std::endl;
}
	std::vector<uint8_t> msg(4, 'a');
	n.sendMsg(msg);
}

int main (int argc, char *argv[])
{

	if (argc == 3) {
		connecter(std::atoi(argv[1]), std::atoi(argv[2]));
	} else if (argc == 2) {
		listener(std::atoi(argv[1]));
	} else {
		throw std::invalid_argument("Invalid number of arguments.");
	}
	return 0;
}
