#pragma once
#include <string>
#include <vector>
#include <iostream>
struct peer_info_s{
	std::vector<char> m_remote_id; // peer id
	std::string m_remote_ip; // remote end of this peer
	std::string m_remote_port;
	std::string m_id; // my id
	std::vector<unsigned char> m_info_hash; // move this out of here to a torrent_info

	peer_info_s() = default;

	peer_info_s(struct peer_info_s && other){
		m_remote_id = std::move(other.m_remote_id);
		m_remote_ip = std::move(other.m_remote_ip);
		m_remote_port = std::move(other.m_remote_port);
		m_id = std::move(other.m_id);
		m_info_hash = std::move(other.m_info_hash);
	}

	peer_info_s(const peer_info_s& other) = default;

	peer_info_s& operator=(const struct peer_info_s & other){
		m_remote_id = other.m_remote_id;
		m_remote_ip = other.m_remote_ip;
		m_remote_port = other.m_remote_port;
		m_id = other.m_id;
		m_info_hash = other.m_info_hash;
		return *this;
	}

};
