#pragma once
#include <vector>
#include "block.h"
#include "info_hash.h"

class peer_connection;

class piece{
	friend bool compare_pieces(const piece& a, const piece& b);
	public:
		piece() = default;
		piece(int index, aux::info_hash piece_hash);
		piece(const piece& other);

		const aux::info_hash& hash() const { return m_piece_hash; };
		piece&operator=(piece && other);
		//piece(int index, const aux::info_hash& piece_hash);
		int index() const { return m_index; };
		int count() const { return m_count; };

		// update list of peers who have this piece
		void add_peer(peer_connection * peer);

		
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
		//each block that make up the piece 16 blocks
		std::vector<block *> m_piece_blocks;
		// the piece hash to verify once we get all the blocks
		aux::info_hash m_piece_hash;
		// blocks received so far
		int m_received_count;
	protected:
		// vector of pointer to the peers who have it
		std::vector<peer_connection *> m_peers;

};
