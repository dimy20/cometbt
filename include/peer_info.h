#pragma once
#include <string>
#include <vector>
struct peer_info_s{
	std::vector<char> m_id; // peer id
	std::string m_remote_ip; // remote end of this peer
	std::string m_port; 
};
