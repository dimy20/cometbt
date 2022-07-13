#pragma once
#include <string>
#include <vector>
struct peer_info_s{
	std::vector<char> m_remote_id; // peer id
	std::string m_remote_ip; // remote end of this peer
	std::string m_remote_port;
	std::string m_id; // my id
	std::vector<unsigned char> m_info_hash; // move this out of here to a torrent_info
};
