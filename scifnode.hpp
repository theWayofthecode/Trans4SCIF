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

#pragma once
#include <system_error>
#include <cstdint>
#include <vector>
#include <memory>
#include "scifepd.hpp"
#include "rmawindow.hpp"

class ScifNode
{
private:
	ScifEpd epd;
	std::vector<std::unique_ptr<RMAWindow>> rmawins;
public:
	ScifNode() = delete;

	/* Construct a connecting node */
	ScifNode(uint16_t target_node_id, uint16_t target_port);

	/* Construct a listening node */
	ScifNode(uint16_t listening_port);

	/* Sends synchronously payload.size() bytes and returns.
		On error it throws a system_error exception */
	void sendMsg(std::vector<uint8_t> &payload);

	/* Receivs synchronously payload.size() bytes and returns.
		On error it throws a system_error exception */
	void recvMsg(std::vector<uint8_t> &payload);

	/* Opens a window for RMA operations in the registered space of the process */
	struct RMAWindowRef openRMAWindow(int num_of_pages, int prot_flags = SCIF_PROT_READ | SCIF_PROT_WRITE);

	/* This method is a simple wrapper arround scif_vwriteto */
	void writeMsg(off_t dest, off_t src, std::size_t len);

	/* This method writes on a remote location the value val 
	to signify the completion of marked RMA operations */
	void signalPeer(off_t dest, std::uint64_t val);
};