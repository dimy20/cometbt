#include "session.h"
#include <pthread.h>

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
	// contacts tracker
	m_torrent.setup_peerinfo();
	m_uv_loop = uv_default_loop();
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
	uv_async_t async;
    uv_async_init(m_uv_loop, &async, try_download);

	// client hadles just one peer for now
	auto piece_len = m_torrent.piece_len();
	m_piece_manager = std::move(piece_manager(piece_len, m_torrent.get_pieces_hash()));

	int n;
	n = m_torrent.get_peers_infos().size();
	for(int i = 0; i < n; i++){
		peer_connection peer(m_torrent.get_peers_infos()[i], &m_piece_manager, &async);
		m_peer_connections.push_back(std::move(peer));
	};

	std::queue<int> work_queue = m_piece_manager.get_work_queue();

	for(auto& peer_conn: m_peer_connections){
		peer_conn.start_v2(m_uv_loop);
	};

	uv_run(m_uv_loop, UV_RUN_DEFAULT);

};
