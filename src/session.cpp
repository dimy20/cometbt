#include "session.h"

static void read_cb(SocketTcp * sock){
	PeerConnection * peer = dynamic_cast<PeerConnection *>(sock);
	peer->on_receive_data();
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

Session::Session(const std::string& filename){
	auto torrent_file = open_file(filename);
	m_torrent.set(std::move(torrent_file));
	m_torrent.init_torrent_data();

	// populates peers infos
	m_torrent.setup_peerinfo();
}

// should only be responsible for handling main loop for now
void Session::start(){
	// client hadles just one peer for now, this will change when buffer layers are
	// added and bandwidth manager is implemented
	
	// add one connection
	m_peer_connections.push_back(PeerConnection(m_torrent.get_peers_infos()[0]));
	auto peer = m_peer_connections[0];
	peer.start();
	// watch one connection
	m_main_loop.watch(&peer, EventLoop::ev_type::READ, read_cb);
	m_main_loop.run();
};
