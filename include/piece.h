#pragma once
#include <vector>
#include <memory>
#include <iostream>
#include "block.h"
#include "info_hash.h"

class peer_connection;

class piece{
	friend bool compare_pieces(const piece& a, const piece& b);
	public:
		piece() = default;
		piece(int index, aux::info_hash piece_hash, long long piece_length);
		piece(const piece& other);

		const aux::info_hash& hash() const { return m_piece_hash; };
		piece&operator=(piece && other);
		//piece(int index, const aux::info_hash& piece_hash);
		int index() const { return m_index; };
		int count() const { return m_count; };
		int length() const { return m_piece_length; };

		// update list of peers who have this piece
		void add_peer(peer_connection * peer);
		bool download_finished() { return m_received_count == 16; };
		bool verify_integrity();
		
		std::vector<peer_connection *>& peers() { return m_peers; };
	public:

	private :
		// piece index
		int m_index;
		// piece length in bytes
		long long m_piece_length;
		// count of how many peers have this piece
		int m_count;
		// each block size
		int m_block_size;
		// the piece hash to verify once we get all the blocks
		aux::info_hash m_piece_hash;
		// blocks received so far
		int m_received_count;
		// received so far
		int m_received_bytes = 0;
		// piece data
		char * m_data = nullptr;
	protected:
		// vector of pointer to the peers who have it
		std::vector<peer_connection *> m_peers;

};
