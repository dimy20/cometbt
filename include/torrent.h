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
#include <stdio.h>

#include "bencode.h"
#include "socketSSL.h"
#include "tcp.h"

#define BUFF_SIZE 1024*16
#define HANDSHAKE_SIZE 68
#define RESERVED_BYTES_LENGTH 8
#define INFO_HASH_LENGTH 20
#define PEER_ID_LENGTH   20
#define PROTOCOL_ID_LENGTH 19
#define PROTOCOL_ID "BitTorrent protocol"

typedef struct info_file_s info_file_t;

struct info_file_s{
	int length;
	std::string path;
};

struct handshake_s{
	char proto_id_size;
	char protocol_id[PROTOCOL_ID_LENGTH];
	char reserved_bytes[RESERVED_BYTES_LENGTH];
	char info_hash[INFO_HASH_LENGTH];
	char peer_id[PEER_ID_LENGTH];
};

class Peer{
	public:

		Peer(std::vector<char> id, const std::string& ip, const std::string& port);
		void send_handshake(const std::vector<unsigned char>& info_hash, const std::string& id);
	public:
		enum p_state{
			HANDSHAKE_SENT = 1,
			HANDSHAKE_DONE = 2
		};
		int m_state;
		std::vector<char> m_id;
		std::string m_ip;
		std::string m_port;
		SocketTcp m_sock;
};

struct peer_s{
	std::vector<char> id;
	std::string ip;
	std::string port;
};

class Torrent{
	public:
		Torrent(const std::string& filename);
		const std::string& get_announce();
		const std::vector<std::string>& get_announce_list();
		const std::vector<info_file_t>& get_info_files();
		const std::vector<Peer>& get_peers();


	private:
		std::string build_request(const std::string& host);
		void init_torrent_data();

	private:
		std::string m_id;						/*this peer id*/
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

		std::vector<unsigned char> m_info_hash;
		std::string m_infohash_hex; /*info dict's formatted sha1 hex for tracker*/

		SocketSSL m_sock;

		std::vector<Peer> m_peers;
};


