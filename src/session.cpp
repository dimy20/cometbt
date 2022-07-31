#include "session.h"

// move to aux?
static std::vector<char> open_file(const std::string& filename){
	std::vector<char> buff;
	int size;

	std::ifstream file(filename, std::ios::binary | std::ios::ate);
	size = file.tellg();
	file.seekg(0, std::ios::beg);
	buff.resize(size);

	file.read(buff.data(), size);
	return buff;
};

Session::Session(const std::string& filename){
	auto torrent_file = open_file(filename);
	m_torrent.set(std::move(torrent_file));
	m_torrent.init_torrent_data();

	// populates peers infos
	m_torrent.setup_peerinfo();
}

// should only be responsible for handling main loop for now
void Session::start(){
	// client hadles just one peer for now
	m_piece_manager = std::move(piece_manager(m_torrent.get_pieces_hash()));
	
	int n;
	n = m_torrent.get_peers_infos().size();
	for(int i = 0; i < n; i++){
		peer_connection peer(m_torrent.get_peers_infos()[i], &m_main_loop,
				&m_piece_manager);

		m_peer_connections.push_back(std::move(peer));
	};

	for(int i = 0; i < m_peer_connections.size(); i++){
		m_peer_connections[i].start();
	}

	m_main_loop.run();
};
