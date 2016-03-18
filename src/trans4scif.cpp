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

#include <iostream>
#include <algorithm>
#include <scif.h>
#include <cassert>
#include <sstream>
#include "constants.hpp"
#include "rmawindow.hpp"
#include "util.hpp"
#include "ctl_messages.hpp"
#include "trans4scif.hpp"
#include "trans4scif_config.hpp"

std::string t4s_version() {
    std::stringstream ss;
    ss << TRANS4SCIF_VERSION_MAJOR << "." << TRANS4SCIF_VERSION_MINOR;
    return ss.str();
}

t4s_socket_t::t4s_socket_t(uint16_t target_node_id, uint16_t target_port) :
        sn(target_node_id, target_port),
        recvbuf(sn.create_RMAWindow(RECV_BUF_SIZE, SCIF_PROT_WRITE)) {
    init();
}

t4s_socket_t::t4s_socket_t(uint16_t listening_port) :
        sn(listening_port),
        recvbuf(sn.create_RMAWindow(RECV_BUF_SIZE, SCIF_PROT_WRITE)) {
    init();
}

void t4s_socket_t::init() {
    /* Send to peer node the recvbuf details */
    RMA_id my_id{recvbuf.get_wr_rmaoff(), recvbuf.get_space()};
    std::vector<uint8_t> msg_send(pack_RMA_id_msg(my_id));
    sn.sendMsg(msg_send);
    /* Get the peer node's recvbuf details */
    std::vector<uint8_t> msg_recv = sn.recvMsg(sizeof(RMA_id));
    RMA_id peer_id(unpack_RMA_id_msg(msg_recv));

    /* Init the rem_recvbuf and sendbuf */
    rem_recvbuf.reset(new Virt_circbuf(peer_id.off, peer_id.size));
    sendbuf.reset(new Circbuf(sn.create_RMAWindow(peer_id.size, SCIF_PROT_READ)));
}


std::size_t t4s_socket_t::send(std::vector<uint8_t>::const_iterator msg_it, std::size_t msg_size) {
    get_rem_recvbuf_notifs();
    if (!rem_recvbuf->get_space()) {
        return 0;
    }

    off_t sync_off = rem_recvbuf->get_wr_rmaoff();
    off_t dest_off = rem_recvbuf->get_wr_rmaoff();
    off_t src_off = sendbuf->get_wr_rmaoff();

    /** Reset the chunk head field to 0. It should be writen by scif_signal*/
    sendbuf->wr_reset_chunk_head();
    rem_recvbuf->wr_advance(CHUNK_HEAD_SIZE);

    /** Write the data */
    auto write_data = [this, &msg_it, &msg_size]() -> std::size_t {
        std::size_t sendbuf_written = sendbuf->write(msg_it, msg_size);
        std::size_t rem_recvbuf_advanced = rem_recvbuf->wr_advance(msg_size);
        assert(sendbuf_written == rem_recvbuf_advanced);
        sendbuf->wr_align();
        rem_recvbuf->wr_align();
        msg_it += sendbuf_written;
        msg_size -= sendbuf_written;
        return sendbuf_written;
    };
    std::size_t written = write_data();

    /** Issue the RDMA write (TODO:cache aligne the end as well) */
    sn.writeMsg(dest_off, src_off, ROUND_TO_BOUNDARY(CHUNK_HEAD_SIZE + written, CACHELINE_SIZE));
    std::size_t total_size_send = written;


    /** Check if we can write some more data (circbuf wrap around case) */
    if (rem_recvbuf->get_space() && msg_size) {
        /** Get the offsets to initiate the RMDA write and synchronization */
        off_t src_off = sendbuf->get_wr_rmaoff();
        off_t dest_off = rem_recvbuf->get_wr_rmaoff();
        std::size_t written = write_data();
        /** Issue the RDMA write (cache aligne the end as well) */
        sn.writeMsg(dest_off, src_off, ROUND_TO_BOUNDARY(written, CACHELINE_SIZE));
        total_size_send += written;
    }

    /** write (remote) chunk head */
    sn.signalPeer(sync_off, total_size_send);

    return total_size_send;
}

/**
 * When the buffer is empty it should block?
 */
std::size_t t4s_socket_t::recv(std::vector<uint8_t>::iterator msg_it, std::size_t msg_size) {
    /** TODO: check if msg_msg_size > recv_buf_size */
    update_recvbuf_space();
    if (recvbuf.is_empty()) {
        return 0;
    }

    /** Currently the chunk header contains only the size of the chunk */
    std::size_t chunk_size = recvbuf.rd_read_reset_chunk_head();

    auto read_data = [this, &msg_size, &msg_it, &chunk_size]() -> std::size_t {
        std::size_t msg_size_read = recvbuf.read(msg_it, std::min(chunk_size, msg_size));
        chunk_size -= msg_size_read;
        msg_size -= msg_size_read;
        msg_it += msg_size_read;
        return msg_size_read;
    };
    std::size_t total_msg_size_recv = read_data();

    if (!recvbuf.is_empty() && chunk_size && msg_size) {
        total_msg_size_recv += read_data();
    }

    /** Truncate */
    /** TODO: consider a warning message here */
    std::size_t truncated_msg_size = 0;
    if (!recvbuf.is_empty() && chunk_size) {
        truncated_msg_size += recvbuf.rd_advance(chunk_size);
    }

    recvbuf.rd_align();

    /** Notify sender */
    std::vector<uint8_t> recv_notif_msg;
    inttype_to_vec_le(total_msg_size_recv + truncated_msg_size, recv_notif_msg);
    sn.sendMsg(recv_notif_msg);

    return total_msg_size_recv;
}

/**
 * When the buffer is empty it should block?
 */
std::vector<uint8_t> t4s_socket_t::recv() {
    update_recvbuf_space();
    if (recvbuf.is_empty()) {
        std::vector<uint8_t> empty_v;
        return empty_v;
    }

    /** Currently the chunk header contains only the size of the chunk */
    std::size_t chunk_size = recvbuf.rd_read_reset_chunk_head();
    std::vector<uint8_t> msg(chunk_size);
    auto msg_it = msg.begin();
    auto msg_size_read = recvbuf.read(msg_it, chunk_size);
    msg_it += msg_size_read;
    chunk_size -= msg_size_read;
    /** wrap-around case */
    if (chunk_size) {
        chunk_size -= recvbuf.read(msg_it, chunk_size);
    }
    assert(!chunk_size);

    recvbuf.rd_align();

    /** Notify sender */
    std::vector<uint8_t> recv_notif_msg;
    inttype_to_vec_le(msg.size(), recv_notif_msg);
    sn.sendMsg(recv_notif_msg);

    /** Assuming copy elision */
    return msg;
}

void t4s_socket_t::update_recvbuf_space() {
    std::size_t chunk_size = 0;
    while (recvbuf.get_space() && (chunk_size = recvbuf.wr_read_chunk_head())) {
        chunk_size -= recvbuf.wr_advance(chunk_size);
        if (recvbuf.get_space() && chunk_size) {
            recvbuf.wr_advance(chunk_size);
        }
        recvbuf.wr_align();
    }
}

void t4s_socket_t::get_rem_recvbuf_notifs() {
    while (sn.has_recv_msg()) {
        std::vector<uint8_t> recv_notif_msg = sn.recvMsg(sizeof(std::size_t));
        std::size_t msg_size;
        vec_to_inttype_le(recv_notif_msg, msg_size);

        std::size_t l = rem_recvbuf->rd_advance(CHUNK_HEAD_SIZE + msg_size);
        assert(l == (CHUNK_HEAD_SIZE + msg_size));
        rem_recvbuf->rd_align();

        l = sendbuf->rd_advance(CHUNK_HEAD_SIZE + msg_size);
        assert(l == (CHUNK_HEAD_SIZE + msg_size));
        sendbuf->rd_align();
    }
}
