#pragma once

#include <iostream>
#include "peer_connection_core.h"
#include "event_loop.h"
#include "peer_info.h"
#include "serial.h"

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
		void do_message(const char * msg_buff, int payload_len);

	private:
		int m_msg_len;
		int m_total;
		char * m_bitfield;
		bool m_choked;
};
