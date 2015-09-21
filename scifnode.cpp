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

#include <system_error>
#include "scifnode.hpp"


ScifNode::ScifNode(uint16_t target_node_id, uint16_t target_port)
{
	struct scif_portID target_addr;
	target_addr.node = target_node_id;
	target_addr.port = target_port;
	if (scif_connect(epd.get_epd_t(), &target_addr) == -1)
		throw std::system_error(errno, std::system_category());
}

ScifNode::ScifNode(uint16_t listening_port)
{
	/* bind */
	if (scif_bind(epd.get_epd_t(), listening_port) == -1)
		throw std::system_error(errno, std::system_category());

	/* listen (backlog = 1) */
	if (scif_listen(epd.get_epd_t(), 1) == -1)
		throw std::system_error(errno, std::system_category());

	/* accept */
	scif_epd_t acc_epd_t;
	struct scif_portID peer_addr;
	if (scif_accept(epd.get_epd_t(), &peer_addr, &acc_epd_t, SCIF_ACCEPT_SYNC) == -1)
		throw std::system_error(errno, std::system_category());
	epd.set_epd_t(acc_epd_t);
}

void ScifNode::sendMsg(std::vector<uint8_t> &payload)
{
	for (int i = 0; i < payload.size();) {
		int bytes = scif_send(epd.get_epd_t(), &payload[i], payload.size() - i, 0);
		if (bytes == -1)
			throw std::system_error(errno, std::system_category());
		i += bytes;
	}
}

void ScifNode::recvMsg(std::vector<uint8_t> &payload)
{
	for (int i = 0; i < payload.size();) {
		int bytes = scif_recv(epd.get_epd_t(), &payload[i], payload.size() - i, 0);
		if (bytes == -1)
			throw std::system_error(errno, std::system_category());
		i += bytes;
	}
}

struct RMAWindowRef ScifNode::openRMAWindow(int num_of_pages, int prot_flags)
{
	rmawins.push_back(
		std::unique_ptr<RMAWindow>(new RMAWindow(epd.get_epd_t(), num_of_pages, prot_flags))
		);
	return rmawins.back()->getRMAWindowRef();
}

void ScifNode::writeMsg(off_t dest, off_t src, std::size_t len)
{
	if (scif_writeto(epd.get_epd_t(), src, len, dest, 0) == -1) {
		throw std::system_error(errno, std::system_category());
	}
}

void ScifNode::signalPeer(off_t dest, std::uint64_t val)
{
	if (scif_fence_signal(epd.get_epd_t(), 0, 0, dest, val, SCIF_FENCE_INIT_SELF | SCIF_SIGNAL_REMOTE) == -1) {
		throw std::system_error(errno, std::system_category());
	}
}