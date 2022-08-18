#include "session.h"
void callback(void){
	std::cout << "THIS IS A CALLBACK " << std::endl;
};
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

session::session(const std::string& filename){
	auto torrent_file = open_file(filename);
	m_torrent.set(std::move(torrent_file));
	m_torrent.init_torrent_data();

	// populates peers infos
	m_torrent.setup_peerinfo();
}

void * io_worker(void * arg){

	event_loop ev_loop;
	session * s = reinterpret_cast<session*>(arg);

	for(auto& peer_conn: s->m_peer_connections){
		peer_conn.start(&ev_loop);
	};

	ev_loop.run();

	return nullptr;
};
// should only be responsible for handling main loop for now
void session::start(){
	// client hadles just one peer for now
	m_piece_manager = std::move(piece_manager(m_torrent.get_pieces_hash()));
	
	int n;
	n = m_torrent.get_peers_infos().size();
	for(int i = 0; i < 1; i++){
		peer_connection peer(m_torrent.get_peers_infos()[i], &m_main_loop,
				&m_piece_manager);

		m_peer_connections.push_back(std::move(peer));
	};

	for(int i = 0; i < m_peer_connections.size(); i++){
		m_peer_connections[i].start();
	}

	timer t(2000, callback);
	m_main_loop.set_timer(t);

	m_main_loop.run();
};
