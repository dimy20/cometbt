#pragma once
#include <vector>
#include <memory>
#include <uv.h>
#include "peer_info.h"
#include "recv_buffer.h"

#define BUFF_SIZE 1024*32

/* handshake macros */
#define HANDSHAKE_SIZE 68
#define RESERVED_BYTES_LENGTH 8
#define INFO_HASH_LENGTH 20
#define PEER_ID_LENGTH   20
#define PROTOCOL_ID_LENGTH 19
#define PROTOCOL_ID "BitTorrent protocol"

/* message macros*/

#define MESSAGE_LENGTH_SIZE 4 // length prefix on every message
#define MESSAGE_ID_SIZE 1
#define BLOCK_LENGTH 1024 * 16
#define PIECE_INDEX_SIZE 4
#define BLOCK_OFFSET_SIZE 4
#define BLOCK_LENGTH_SIZE 4

enum class message_id{
	CHOKE = 0,
	UNCHOKE,
	INTERESTED,
	NOTINTERESTED,
	HAVE,
	BITFIELD,
	REQUEST,
	PIECE,
	CANCEL
};

struct have_message{
	std::uint8_t length[MESSAGE_LENGTH_SIZE];
	std::uint8_t id = static_cast<std::uint8_t>(message_id::HAVE);
	std::uint8_t index[sizeof(int)];
};

struct interested_message{
	std::uint8_t length[MESSAGE_LENGTH_SIZE];
	std::uint8_t id = static_cast<std::uint8_t>(message_id::INTERESTED);
};


struct req_message{
	std::uint8_t length[MESSAGE_LENGTH_SIZE]; /* prefix length*/
	std::uint8_t id;						 /* message id aka REQUEST*/
	std::uint8_t index[PIECE_INDEX_SIZE];    // index of the piece we're requesting
	std::uint8_t block_offset[BLOCK_OFFSET_SIZE]; // block offset within the piece
	std::uint8_t block_length[BLOCK_LENGTH_SIZE];  // length of the block
};

class peer_connection_core{
	public:
		peer_connection_core(const struct peer_info_s& peer);
		peer_connection_core(peer_connection_core && other);
		~peer_connection_core();
		void send_handshake(const aux::info_hash& info_hash, const std::string& id);

		// starts connection and prepares receive buffer
		void start(uv_loop_t * loop);

		friend void on_read(uv_stream_t * tcp, ssize_t nread, const uv_buf_t * buf);
		friend void handshake_write_cb(uv_write_t *req, int status);
		friend void on_connect(uv_connect_t * req, int status);

		virtual void on_receive(int passed_bytes) = 0;
		virtual void on_remote_close() = 0;

	private:
		void on_receive_internal(int received_bytes);

	protected:
		void setup_receive();
		uv_stream_t * socket() const { return m_socket; };

	protected:
		enum class p_state{
			READ_PROTOCOL_ID,
			READ_INFO_HASH,
			READ_PEER_ID,
			READ_MESSAGE_SIZE,
			READ_MESSAGE,
			NOT_IMPLEMENTED_YET
		};

		recv_buffer m_recv_buffer;
		bool m_disconnect = false;
		uv_loop_t * m_loop = nullptr;
		p_state m_state;
		struct peer_info_s m_peer_info;
		uv_stream_t * m_socket = nullptr;
};

struct handshake_s{
	char proto_id_size;
	char protocol_id[PROTOCOL_ID_LENGTH];
	char reserved_bytes[RESERVED_BYTES_LENGTH];
	char info_hash[INFO_HASH_LENGTH];
	char peer_id[PEER_ID_LENGTH];
};
