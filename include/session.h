#pragma once
#include <vector>
#include "peer_connection.h"
#include "torrent.h"
#include "piece_manager.h"
#include <uv.h>
// class responsible for managing the peer connections
// holds event loop
// this class will stand high in the layer hierarchy
// we will probably have some other session class reponsible for working with peer's
// socket which this class will derive from, for now we will keep everything coupled
// together here until i decide to abstract down this functionality as the class
// grows.
// Peer connection need peer_info to populate its internal fields, how to get
// that from torrent?
// session now only supports one torrent and one set of peers for that torrent
class peer_connection;
class session{
		friend void * io_worker(void * arg);
	public:
		session(const std::string& filename);
		void start();
//	private:
		torrent m_torrent; // make this better
		std::vector<peer_connection> m_peer_connections;
		piece_manager m_piece_manager;
		uv_loop_t * m_uv_loop;
};
