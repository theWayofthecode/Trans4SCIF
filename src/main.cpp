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

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <string>
#include <cstring>
#include <chrono>
#include <thread>
#include <utility>
#include "scifnode.hpp"
#include "trans4node.hpp"
#include "virt_circbuf.hpp"
#include "circbuf.hpp"
#include "util.hpp"

void test_virt_circbuf ()
{
	Virt_circbuf vc(0xff, 0x1000);

	std::cout << vc.get_space() << std::endl;
	std::size_t written = vc.write(60);
	std::cout << vc.get_space() <<std::endl << written << std::endl;
}

void test_circbuf(ScifNode& n)
{
	RMAWindow w = n.createRMAWindow(1);
	Circbuf cb(w);
	std::vector<uint8_t> v{0xD, 0xB, 0xC};
	std::cout << cb.get_space() << std::endl;
	std::cout << cb.write(50) << std::endl;
	std::cout << cb.write(v) << std::endl;
	std::cout << cb.write(5000) << std::endl;
	std::cout << cb.get_space() << std::endl;

	std::vector<uint8_t> vread(3);
	std::cout << cb.read(40) << std::endl;
	std::cout << cb.read(3) << std::endl;
	std::cout << cb.read(5000) << std::endl;
	std::cout << cb.write(v) << std::endl;
	std::cout << cb.read(vread) << std::endl;
	std::cout << cb.get_space() << std::endl;
	std::cout << std::hex << static_cast<int>(vread.front()) << std::endl;
}

void connecter (uint16_t target_node_id, uint16_t target_port)
{
	Trans4Node tn(target_node_id, target_port);
}

void connecter_old (uint16_t target_node_id, uint16_t target_port)
{
	ScifNode n(target_node_id, target_port);
//	Trans4Node tn(target_node_id, target_port+1);
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));


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

	//test virt_circbuf
	//test_virt_circbuf();
}

void listener (uint16_t listening_port)
{
	Trans4Node tn(listening_port);
}

void listener_old (uint16_t listening_port)
{
	ScifNode n(listening_port);

	//receive remote offset
	std::vector<uint8_t> vroff(sizeof(off_t));
	n.recvMsg(vroff);
	off_t roff = *(reinterpret_cast<off_t *>(vroff.data()));

	//open a window and send the local offset
	RMAWindow win = n.createRMAWindow(1);
	std::cout << win.get_len() << std::endl << std::endl;
	off_t off = win.get_off();
	uint8_t *p = reinterpret_cast<uint8_t *>(&off);
	std::vector<uint8_t> v(p, p + sizeof(off_t));
	n.sendMsg(v);

	//Test circbuf
	test_circbuf(n);

	//writeMsg
	char *mem = static_cast<char *>(win.get_mem());
	std::memcpy (mem+64, "Hallo", sizeof("Hallo"));
	n.writeMsg(roff + 64, win.get_off() + 64, sizeof("Hallo"));
	n.signalPeer(roff, sizeof("Hallo"));
}

void test_util()
{
	uint64_t i = 0xabcabcabc;
	uint64_t j = 0;
	std::vector<uint8_t> v;
	inttype_to_vec_le(i, v);
	vec_to_inttype_le(v, j);
	std::cout << std::hex << j << std::endl;
}


int main (int argc, char *argv[])
{
	test_util();
	if (argc == 3) {
		connecter(std::atoi(argv[1]), std::atoi(argv[2]));
	} else if (argc == 2) {
		listener(std::atoi(argv[1]));
	} else {
		throw std::invalid_argument("Invalid number of arguments.");
	}
	return 0;
}
