#pragma once

#include <iostream>
#include "peer_connection_core.h"
#include "peer_info.h"
#include "serial.h"
#include "bitfield.h"
#include "piece_manager.h"
#include "piece.h"
#include <uv.h>

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
		int fetch_piece(int index, std::queue<int>& work_queue);

		bool choked() { return m_choked; };
	private:
		int get_length(const char * const buff, std::size_t size);
		//change this to work directly with the receive buffer
		void do_message();

		// message handlers
		void handle_bitfield();
		void handle_unchoke();
		void handle_choke();
		void handle_piece();

	public:
		piece_manager * m_piece_manager;
		int m_msg_len;
		int m_total;
		bool m_choked;
		aux::bitfield m_bitfield;
		uv_async_t * m_async;
		int m_curr_piece = -1;
};

