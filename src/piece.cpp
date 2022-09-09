#include "piece.h"

piece::piece(int index, aux::info_hash piece_hash, long long piece_length){
	m_index = index;
	m_piece_hash = piece_hash;
	m_count = 0;
	m_block_size = 1024 * 16;
	m_piece_length = piece_length;
	m_received_count = 0;
	m_data = static_cast<char*>(std::malloc(sizeof(char) * piece_length));
	if(!m_data) throw(std::bad_alloc());
	memset(m_data, 0, m_piece_length);
	m_received_bytes = 0;
};

piece::piece(const piece& other){
	 m_index = other.m_index; // piece index
	 m_piece_length = other.m_piece_length;
	 m_count = other.m_count;// count of how many peers have this piece
	 m_block_size = other.m_block_size;
	 // each block that make up the piece 16 blocks
	 m_piece_hash = other.m_piece_hash;
	 m_peers = other.m_peers;
	 m_received_count = other.m_received_count;
	 memcpy(m_data, other.m_data, m_piece_length);
	 m_received_bytes = other.m_received_bytes;
};

piece& piece::operator=(piece && other){
	m_index = other.m_index;
	m_piece_length = other.m_piece_length;
	m_count = other.m_count;
	m_block_size = other.m_block_size;
	//m_peers = std::move(other.m_peers);
	m_piece_hash = std::move(other.m_piece_hash);
	m_received_count = other.m_received_count;

	m_data = other.m_data;
	other.m_data = nullptr;
	m_received_bytes = other.m_received_bytes;
	return *this;
};

void piece::add_peer(peer_connection * peer){
	m_peers.push_back(peer);
	m_count++;
};

bool piece::verify_integrity(){
	aux::info_hash received_hash(m_data, m_received_bytes);
	return m_piece_hash == received_hash;
};

void piece::incoming_block(block_t& b){
	m_received_bytes += b.m_size;
	memcpy(m_data + b.m_offset, b.m_data, b.m_size);
	log_progress(b.m_size);
	m_received_count++;
};

void piece::log_progress(int received){
	int bar_width;
	if(m_progress < 1.0) {
		int bar_width = 70;

		std::cout << "[";
		int pos = bar_width * m_progress;
		for (int i = 0; i < bar_width; ++i) {
			if (i < pos) std::cout << "=";
			else if (i == pos) std::cout << ">";
			else std::cout << " ";
		}
		std::cout << "] ";
		int current_kb = m_received_bytes / 1024;
		int total_kb = m_piece_length / 1024;
		std::cout << " " << current_kb << " kb / " << total_kb << " kb\r";
		std::cout.flush();

		float tmp = (float)(received * 100) / (float)(256 * 1024); 
		m_progress += (float)(tmp / 100);
	}else{
		std::cout << std::endl;
	}
}

void piece::reset(){
	m_progress = 0.0f;
	m_received_count = 0;
	m_backlog = 0;
	m_total_requested = 0;
	memset(m_data, 0, m_received_bytes);
	m_received_bytes = 0;
};
