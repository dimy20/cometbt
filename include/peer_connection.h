#pragma once

#include <iostream>
#include "peer_connection_core.h"
#include "event_loop.h"
#include "peer_info.h"
#include "serial.h"
#include "bitfield.h"

// this class implements the protocol
class peer_connection : public PeerConnectionCore{

	public:
		peer_connection();
		peer_connection(const struct peer_info_s& peer, EventLoop * loop);
		virtual void on_receive(int passed_bytes);
	private:
		int get_length(const char * const buff, std::size_t size);
		bool has_piece(int index);
		//change this to work directly with the receive buffer
		void do_message();

		// message handlers
		void handle_bitfield(char * begin, std::size_t size);

	private:
		int m_msg_len;
		int m_total;
		bool m_choked;
		aux::bitfield m_bitfield;
};
