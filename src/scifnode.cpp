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

#include <system_error>
#include "scifnode.hpp"
#include "constants.hpp"


ScifNode::ScifNode(uint16_t target_node_id, uint16_t target_port)
{
	/* connect */
	struct scif_portID target_addr;
	target_addr.node = target_node_id;
	target_addr.port = target_port;
	if (scif_connect(epd.get_epd_t(), &target_addr) == -1)
		throw std::system_error(errno, std::system_category(), __FILE__LINE__);
}

ScifNode::ScifNode(uint16_t listening_port)
{
	/* bind */
	if (scif_bind(epd.get_epd_t(), listening_port) == -1)
		throw std::system_error(errno, std::system_category(), __FILE__LINE__);

	/* listen (backlog = 1) */
	if (scif_listen(epd.get_epd_t(), 1) == -1)
		throw std::system_error(errno, std::system_category(), __FILE__LINE__);

	/* accept */
	scif_epd_t acc_epd_t;
	struct scif_portID peer_addr;
	if (scif_accept(epd.get_epd_t(), &peer_addr, &acc_epd_t, SCIF_ACCEPT_SYNC) == -1)
		throw std::system_error(errno, std::system_category(), __FILE__LINE__);
	epd.set_epd_t(acc_epd_t);
}

/** TODO: What if scif_send sends less than payload.size()? */
std::size_t ScifNode::sendMsg(std::vector<uint8_t> &payload)
{
	int bytes = scif_send(epd.get_epd_t(), payload.data(), payload.size(), SCIF_SEND_BLOCK);
	/* TODO: Maybe in case of error earlier return? */
	if (bytes == -1)
		throw std::system_error(errno, std::system_category(), __FILE__LINE__);
	
	return bytes;
}

std::vector<uint8_t> ScifNode::recvMsg(std::size_t size)
{
	std::vector<uint8_t> payload(size);
	int bytes = scif_recv(epd.get_epd_t(), payload.data(), size, SCIF_RECV_BLOCK);
	/* TODO: Maybe in case of error earlier return? */
	if (bytes == -1)
		throw std::system_error(errno, std::system_category(), __FILE__LINE__);
	return payload;
}

void ScifNode::writeMsg(off_t dest, off_t src, std::size_t len)
{
	if (scif_writeto(epd.get_epd_t(), src, len, dest, 0) == -1) {
		throw std::system_error(errno, std::system_category(), __FILE__LINE__);
	}
}

void ScifNode::signalPeer(off_t dest, std::uint64_t val)
{
	if (scif_fence_signal(epd.get_epd_t(), 0, 0, dest, val, SCIF_FENCE_INIT_SELF | SCIF_SIGNAL_REMOTE) == -1) {
		throw std::system_error(errno, std::system_category(), __FILE__LINE__);
	}
}

bool ScifNode::has_recv_msg()
{
	struct scif_pollepd pepd;
	pepd.epd = epd.get_epd_t();
	pepd.events = SCIF_POLLIN;
	pepd.revents = 0;
	if (scif_poll(&pepd, 1, 0) == -1) {
		throw std::system_error(errno, std::system_category(), __FILE__LINE__);		
	}
	return pepd.revents == SCIF_POLLIN;
}