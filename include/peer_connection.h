#pragma once

#include <iostream>
#include "peer_connection_core.h"
#include "event_loop.h"
#include "peer_info.h"
#include "serial.h"
#include "bitfield.h"
#include "piece_manager.h"

class piece_manager;
// this class implements the protocol
class peer_connection : public peer_connection_core{
	public:
		peer_connection();
		peer_connection(const struct peer_info_s& peer, event_loop * loop,
				piece_manager * pm);
		peer_connection(const struct peer_info_s& peer, piece_manager * pm);
		peer_connection(peer_connection && other);
		virtual void on_receive(int passed_bytes);
	private:
		int get_length(const char * const buff, std::size_t size);
		//change this to work directly with the receive buffer
		void do_message();

		// message handlers
		void handle_bitfield(char * begin, std::size_t size);
		void handle_unchoke();
		void handle_choke();
		void handle_piece();

	private:
		piece_manager * m_piece_manager;
		int m_msg_len;
		int m_total;
		bool m_choked;
		aux::bitfield m_bitfield;
};
