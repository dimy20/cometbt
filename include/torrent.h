#pragma once

#include <vector>
#include <string>
#include <fstream>

#include <openssl/sha.h>
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <memory>

#include "bencode.h"
#include "socketSSL.h"
#include "tcp.h"
#include "event_loop.h"
#include "peer_connection.h"

typedef struct info_file_s info_file_t;

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

struct info_file_s{
	int length;
	std::string path;
};



class Torrent{
	public:
		Torrent(const std::string& filename);
		const std::string& get_announce();
		const std::vector<std::string>& get_announce_list();
		const std::vector<info_file_t>& get_info_files();
		const std::vector<PeerConnection> get_peers();
		void download_file();
	public:

		std::vector<unsigned char> m_info_hash;
		std::string m_id;						/*this peer id*/

	private:
		std::string build_request(const std::string& host);
		void init_torrent_data();

	private:

		std::vector<char> m_buff;               /*binary bencode*/
		std::string m_announce;

		/*optional params*/
		std::vector<std::string> m_announce_list; /*optional*/
		std::string m_comment;					  /*optional*/
		std::string m_created_by;                 /*optional*/
		long long m_creation_date;                /*optional*/
		std::string m_encoding;                   /*optional*/


		Bencode::dict_t m_info;
		int m_info_private;                       /*optional*/

		std::vector<info_file_t> m_info_files; /*only in multiple file mode*/

		std::string m_info_name; /*single file mode*/
		long long m_length;

		long long m_info_piecelen; /*size in bytes of each piece*/
		std::vector<char> m_info_pieces; /*piece's sha1 hash*/


		std::string m_infohash_hex; /*info dict's formatted sha1 hex for tracker*/

		SocketSSL m_sock;

		std::vector<PeerConnection> m_peers;

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
