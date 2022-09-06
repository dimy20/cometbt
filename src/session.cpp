#include "session.h"
#include "log.h"

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

bool write_piece(const std::string& filename, piece * p){
	std::ofstream output("test", std::ios::binary | std::ios_base::app);
	if(!output){
		std::cerr << "failed to open filename : " << filename << std::endl;
		return false;
	}
	const char * data = p->data();
	output.write(data, p->size());
	if(output.bad()) return false;
	return true;
};

void save_piece(uv_work_t * handle){
	//std::cout << "Saving piece" << std::endl;
	struct piece_write_req * req = (struct piece_write_req *)handle->data;
	int index = req->index;
	piece_manager * p_manager = req->p_manager;
	auto& piece = p_manager->get_piece(index);
	write_piece("caca", &piece);
};

void after(uv_work_t * req, int status){
	free(req->data);
	free(req);
};
// should only be responsible for handling main loop for now
// maybe i can have a thread to select which peers to run for a given piece?
void try_download(uv_async_t * handle){
	struct send_params * params = (struct send_params*)handle->data;
	assert(params != nullptr);

	peer_connection * peer = params->peer;
	if(!peer->m_bitfield.has_piece(params->index)){
		params->work_queue->push(params->index);
		free(params);
		return;
	}
	if(peer->choked()){
		params->work_queue->push(params->index);
		free(params);
		return;
	}

	int index = peer->fetch_piece(params->index, *params->work_queue);

	if(index != -1){
		uv_work_t * req = (uv_work_t *)malloc(sizeof(uv_work_t));
		COMET_ASSERT_ALLOC(req);
		struct piece_write_req * pwr = (struct piece_write_req *)malloc(sizeof(struct piece_write_req));
		COMET_ASSERT_ALLOC(pwr);

		pwr->index = index;
		pwr->p_manager = params->p_manager;
		req->data = (void *)pwr;

		assert(params->loop != nullptr);
		uv_queue_work(params->loop, req, save_piece, after);

		int next_piece = params->work_queue->front();
		params->work_queue->pop();
		peer->fetch_piece(next_piece, *params->work_queue);
	};
	free(params);
};


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
		peer_conn.start(m_uv_loop);
	};

	uv_run(m_uv_loop, UV_RUN_DEFAULT);

};
