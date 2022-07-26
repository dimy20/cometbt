#include <iostream>
#include "piece_manager.h"

piece::piece(int index, aux::info_hash piece_hash){
	m_index = index;
	m_piece_hash = piece_hash;
};

piece_manager::piece_manager(std::vector<char>& piece_hashes){  
	int n = piece_hashes.size() / 20 ;
	m_pieces = std::vector<piece>(n);

	int offset = 0;
	// build pieces base
	for(int i = 0; i < n; i++){
		aux::info_hash piece_hash(piece_hashes.data() + offset, SHA_DIGEST_LENGTH);
		m_pieces[i] = piece(i, std::move(piece_hash));
		offset += SHA_DIGEST_LENGTH;
	}

	for(auto piece : m_pieces){
		const auto& hash = piece.hash();
		std::cout << hash.hex_str() << std::endl;
	}

};

piece_manager& piece_manager::operator=(piece_manager && other){
	m_pieces = std::move(other.m_pieces);
	return *this;
};
