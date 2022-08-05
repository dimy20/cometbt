#include "piece.h"

piece::piece(int index, aux::info_hash piece_hash){
	m_index = index;
	m_piece_hash = piece_hash;
	m_count = 0;
	m_block_size = 1024 * 16;
};

piece::piece(const piece& other){
	 m_index = other.m_index; // piece index
	 m_piece_length = other.m_piece_length;
	 m_count = other.m_count;// count of how many peers have this piece
	 m_block_size = other.m_block_size;
	 // each block that make up the piece 16 blocks
	 m_piece_blocks = other.m_piece_blocks;
	 m_piece_hash = other.m_piece_hash;
};

piece& piece::operator=(piece && other){
	m_index = other.m_index;
	m_piece_length = other.m_piece_length;
	m_count = other.m_count;
	m_block_size = other.m_block_size;
	//m_peers = std::move(other.m_peers);
	m_piece_blocks = std::move(other.m_piece_blocks);
	m_piece_hash = std::move(other.m_piece_hash);
	return *this;
};
