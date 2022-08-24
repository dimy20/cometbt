#include <iostream>
#include "piece_manager.h"

piece_manager::piece_manager(long long piece_length, std::vector<char>& piece_hashes){
	int n = piece_hashes.size() / 20 ;
	m_pieces = std::vector<piece>(n);

	int offset = 0;
	// build pieces base
	for(int i = 0; i < n; i++){
		aux::info_hash piece_hash;
		piece_hash.set(piece_hashes.data() + offset, SHA_DIGEST_LENGTH);
		m_pieces[i] = std::move(piece(i, std::move(piece_hash), piece_length));
		offset += SHA_DIGEST_LENGTH;
	}

	// build work queue
	for(auto& piece : m_pieces){
		m_work_queue.push(&piece);
	}

};

piece_manager& piece_manager::operator=(piece_manager && other){
	m_pieces = std::move(other.m_pieces);
	m_work_queue = std::move(other.m_work_queue);
	return *this;
};

piece& piece_manager::get_piece(int index){
	int n = m_pieces.size();
	assert(index >= 0 && index <= n);
	return m_pieces[index];
}
