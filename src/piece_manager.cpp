#include <iostream>
#include "piece_manager.h"

piece::piece(int index, aux::info_hash piece_hash){
	m_index = index;
	m_piece_hash = piece_hash;
piece::piece(const piece& other){
	 m_index = other.m_index; // piece index
	 m_piece_length = other.m_piece_length;
	 m_count = other.m_count;// count of how many peers have this piece
	 m_block_size = other.m_block_size;
	 m_blocks = other.m_blocks; // each block that make up the piece 16 blocks
	 m_piece_hash = other.m_piece_hash;
};

piece& piece::operator=(piece && other){
	m_index = other.m_index;
	m_piece_length = other.m_piece_length;
	m_count = other.m_count;
	m_block_size = other.m_block_size;
	m_peers = std::move(other.m_peers);
	m_blocks = std::move(other.m_blocks);
	m_piece_hash = std::move(other.m_piece_hash);
	return *this;
};

piece_manager::piece_manager(std::vector<char>& piece_hashes){  
	int n = piece_hashes.size() / 20 ;
	m_pieces = std::vector<piece>(n);

	int offset = 0;
	// build pieces base
	for(int i = 0; i < n; i++){
		aux::info_hash piece_hash(piece_hashes.data() + offset, SHA_DIGEST_LENGTH);
		m_pieces[i] = std::move(piece(i, std::move(piece_hash)));
		offset += SHA_DIGEST_LENGTH;
	}

	for(const auto& piece : m_pieces){
		const auto& hash = piece.hash();
		std::cout << hash.hex_str() << std::endl;
	}

};

piece_manager& piece_manager::operator=(piece_manager && other){
	m_pieces = std::move(other.m_pieces);
	return *this;
};
