#pragma once
#include "tcp.h"
#include <vector>
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

class PeerConnection : public SocketTcp{
	public:
		PeerConnection(std::vector<char> id, const std::string& ip, const std::string& port);
		void send_handshake(const std::vector<unsigned char>& info_hash, const std::string& id);
		bool wait_handshake();
		bool has_piece(int index);
public:
	enum class p_state{
		HANDSHAKE_WAIT = 1, /*handshake sent and waiting for response*/
		HANDSHAKE_DONE = 2,  /*Successful handshake response received*/
		HANDSHAKE_FAIL = 4,/*Incorrenct handshake response received*/
		MESSAGE_HANDLING = 8,
		MESSAGE_FINISHED = 9
	};
	p_state m_state;
	std::vector<char> m_id;
	std::string m_ip;
	std::string m_port;
	std::vector<unsigned char> m_info_hash;
	// maybe make a message class
	int m_msg_len;
	int m_total;
	char m_buff[BUFF_SIZE];
	char * m_bitfield;
	bool m_choked;
};

struct handshake_s{
	char proto_id_size;
	char protocol_id[PROTOCOL_ID_LENGTH];
	char reserved_bytes[RESERVED_BYTES_LENGTH];
	char info_hash[INFO_HASH_LENGTH];
	char peer_id[PEER_ID_LENGTH];
};
