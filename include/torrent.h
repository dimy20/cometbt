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
#include "peer_connection_core.h"
#include "peer_info.h"
#include "http_parser.h"
#include "info_hash.h"


typedef struct info_file_s info_file_t;


struct info_file_s{
	int length;
	std::string path;
};


class torrent{
	public:
		torrent();
		torrent(const std::string& filename);
		const std::string& get_announce();
		const std::vector<std::string>& get_announce_list();
		const std::vector<info_file_t>& get_info_files();
		void setup_peerinfo();
		void init_torrent_data();
		void set(std::vector<char> && torrent);
		const std::vector<struct peer_info_s>& get_peers_infos(){return m_peers_info;};
		std::vector<char>& get_pieces_hash() { return m_info_pieces; };
		long long piece_len(){ return m_info_piecelen; };

	public:

		aux::info_hash m_info_hash;
		std::string m_id;						/*this peer id*/


	private:

		std::vector<char> m_buff;               /*binary bencode*/
		std::string m_announce;

		/*optional params*/
		std::vector<std::string> m_announce_list; /*optional*/
		std::string m_comment;					  /*optional*/
		std::string m_created_by;                 /*optional*/
		long long m_creation_date;                /*optional*/
		std::string m_encoding;                   /*optional*/


		bencode::dict_t m_info;
		int m_info_private;                       /*optional*/

		std::vector<info_file_t> m_info_files; /*only in multiple file mode*/

		std::string m_info_name; /*single file mode*/
		long long m_length;

		long long m_info_piecelen; /*size in bytes of each piece*/
		std::vector<char> m_info_pieces; /*piece's sha1 hash*/

		SocketSSL m_sock;

		std::vector<struct peer_info_s> m_peers_info;

};
