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

#pragma once

#include <vector>
#include <cstdint>
#include "../src/scifnode.hpp"
#include "../src/virt_circbuf.hpp"
#include "../src/circbuf.hpp"

std::string t4s_version();

class t4s_socket_t {
private:
    ScifNode sn;
    Circbuf recvbuf;
    std::unique_ptr<Circbuf> sendbuf;
    /**
     * Remote (peer) receive buffer. It is virtual in the sense
     * that it is not backed up by memory but used only for the logistics.
     */
    std::unique_ptr<Virt_circbuf> rem_recvbuf;

    void init();

    void get_rem_recvbuf_notifs();

    void update_recvbuf_space();

public:

    /* Construct a connecting node */
    t4s_socket_t(uint16_t target_node_id, uint16_t target_port);

    /* Construct a listening node */
    t4s_socket_t(uint16_t listening_port);

    std::size_t send(std::vector<uint8_t>::const_iterator msg_it, std::size_t len);

    std::size_t recv(std::vector<uint8_t>::iterator msg_it, std::size_t msg_size);

    std::vector<uint8_t> recv();
};