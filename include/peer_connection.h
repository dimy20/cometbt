#pragma once

#include <iostream>
#include "peer_connection_core.h"
#include "event_loop.h"
#include "peer_info.h"
#include "serial.h"
#include "bitfield.h"
#include "piece_manager.h"

#define MAX_BACKLOG 5 // pipeline messages

class piece_manager;
class piece;
// this class implements the protocol
class peer_connection : public peer_connection_core{
	public:
		peer_connection();
		peer_connection(const struct peer_info_s& peer, piece_manager * pm);
		peer_connection(peer_connection && other);
		peer_connection& operator=(const peer_connection& other);

		virtual void on_receive(int passed_bytes);
		void fetch_piece(piece& p);

		bool choked() { return m_choked; };
	private:
		int get_length(const char * const buff, std::size_t size);
		//change this to work directly with the receive buffer
		void do_message();

		// message handlers
		void handle_bitfield(char * begin, std::size_t size);
		void handle_unchoke();
		void handle_choke();
		void handle_piece();


	public:
		piece_manager * m_piece_manager;
		int m_msg_len;
		int m_total;
		bool m_choked;
		aux::bitfield m_bitfield;
		// keeps track of the amount of pipelined requests
		int m_backlog;
		// keeps track of the amount of bytes that have been requested so far
		// for the piece we're currently fecthing from this peer.
		int m_total_requested;
};

