#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <string>
#include <cstring>
#include <chrono>
#include <thread>
#include <utility>
#include "scifnode.hpp"
//#include "trans4node.hpp"

void connecter (uint16_t target_node_id, uint16_t target_port)
{
	ScifNode n(target_node_id, target_port);
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
//	Trans4Node tn(target_node_id, target_port+1);


	//open a window and send the local offset
	RMAWindow win = n.createRMAWindow(1);
	off_t off = win.get_off();
	uint8_t *p = reinterpret_cast<uint8_t *>(&off);
	std::vector<uint8_t> v(p, p + sizeof(win.get_mem()));
	n.sendMsg(v);

	//receive remote offset
	std::vector<uint8_t> vroff(sizeof(off_t));
	n.recvMsg(vroff);
	off_t roff = *(reinterpret_cast<off_t *>(vroff.data()));

	//poll
	uint64_t *mem = static_cast<uint64_t *>(win.get_mem());
	while (*mem != sizeof("Hallo"));
	char *s = static_cast<char *>(win.get_mem());
	std::cout << std::string(s+64, sizeof("Hallo")) << std::endl;
}

void listener (uint16_t listening_port)
{
	ScifNode n(listening_port);
//	Trans4Node tn(listening_port+1);

	//receive remote offset
	std::vector<uint8_t> vroff(sizeof(off_t));
	n.recvMsg(vroff);
	off_t roff = *(reinterpret_cast<off_t *>(vroff.data()));

	//open a window and send the local offset
	RMAWindow win = n.createRMAWindow(1);
	off_t off = win.get_off();
	uint8_t *p = reinterpret_cast<uint8_t *>(&off);
	std::vector<uint8_t> v(p, p + sizeof(off_t));
	n.sendMsg(v);

	//writeMsg
	char *mem = static_cast<char *>(win.get_mem());
	std::memcpy (mem+64, "Hallo", sizeof("Hallo"));
	n.writeMsg(roff + 64, win.get_off() + 64, sizeof("Hallo"));
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
