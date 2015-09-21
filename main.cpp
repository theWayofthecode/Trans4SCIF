#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <string>
#include <cstring>
#include "scifnode.hpp"
#include "rmawindowref.hpp"

void connecter (uint16_t target_node_id, uint16_t target_port)
{
	ScifNode n(target_node_id, target_port);

	//open a window and send the local offset
	auto winref = n.openRMAWindow(1);
	uint8_t *p = reinterpret_cast<uint8_t *>(&winref.off);
	std::vector<uint8_t> v(p, p + sizeof(winref.off));
	n.sendMsg(v);

	//receive remote offset
	std::vector<uint8_t> vroff(sizeof(off_t));
	n.recvMsg(vroff);
	off_t roff = *(reinterpret_cast<off_t *>(vroff.data()));

	//poll
	uint64_t *mem = static_cast<uint64_t *>(winref.mem);
	while (*mem != sizeof("Hallo"));
	char *s = static_cast<char *>(winref.mem);
	std::cout << std::string(s+64, sizeof("Hallo")) << std::endl;
}

void listener (uint16_t listening_port)
{
	ScifNode n(listening_port);

	//receive remote offset
	std::vector<uint8_t> vroff(sizeof(off_t));
	n.recvMsg(vroff);
	off_t roff = *(reinterpret_cast<off_t *>(vroff.data()));

	//open a window and send the local offset
	auto winref = n.openRMAWindow(1);
	uint8_t *p = reinterpret_cast<uint8_t *>(&winref.off);
	std::vector<uint8_t> v(p, p + sizeof(winref.off));
	n.sendMsg(v);

	//writeMsg
	char *mem = static_cast<char *>(winref.mem);
	std::memcpy (mem+64, "Hallo", sizeof("Hallo"));
	n.writeMsg(roff + 64, winref.off + 64, sizeof("Hallo"));
	n.signalPeer(roff, sizeof("Hallo"));
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
