#include "piece.h"

piece::piece(int index, aux::info_hash piece_hash, long long piece_length){
	m_index = index;
	m_piece_hash = piece_hash;
	m_count = 0;
	m_block_size = 1024 * 16;
	m_piece_length = piece_length;
	m_piece_blocks = std::vector<block *>(piece_length / m_block_size, nullptr);
	m_received_count = 0;
};

piece::piece(const piece& other){
	 m_index = other.m_index; // piece index
	 m_piece_length = other.m_piece_length;
	 m_count = other.m_count;// count of how many peers have this piece
	 m_block_size = other.m_block_size;
	 // each block that make up the piece 16 blocks
	 m_piece_blocks = other.m_piece_blocks;
	 m_piece_hash = other.m_piece_hash;
	 m_peers = other.m_peers;
	 m_received_count = other.m_received_count;
};

piece& piece::operator=(piece && other){
	m_index = other.m_index;
	m_piece_length = other.m_piece_length;
	m_count = other.m_count;
	m_block_size = other.m_block_size;
	//m_peers = std::move(other.m_peers);
	m_piece_blocks = std::move(other.m_piece_blocks);
	m_piece_hash = std::move(other.m_piece_hash);
	m_received_count = other.m_received_count;
	return *this;
};

void piece::add_peer(peer_connection * peer){
	m_peers.push_back(peer);
	m_count++;
};

void piece::add_block(block * b){
	assert(b->index() == m_index);
	int i = b->offset() / (16 * 1024);
	int n = m_piece_blocks.size();

	assert(i <= n-1);
	assert(m_received_count <= n);

	if(m_piece_blocks[i] != nullptr){
		std::cerr << "Error : Already received this block" << std::endl;
		exit(EXIT_FAILURE);
	}
	
	m_piece_blocks[i] = b;
	m_received_count++;
}
