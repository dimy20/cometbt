#pragma once
#include <vector>
#include <queue>
#include "info_hash.h"
#include "bitfield.h"
#include "peer_connection.h"

class peer_connection;

class piece{
	friend class piece_manager;
	public:
		piece() = default;
		piece(int index, aux::info_hash piece_hash);
		piece(const piece& other);

		const aux::info_hash& hash() const { return m_piece_hash; };
		piece&operator=(piece && other);
		//piece(int index, const aux::info_hash& piece_hash);
		int index() const { return m_index; };
	private:
		int m_index; // piece index
		int m_piece_length;
		int m_count; // count of how many peers have this piece
		int m_block_size;
		std::vector<void *> m_blocks; // each block that make up the piece 16 blocks
		aux::info_hash m_piece_hash; // 
	protected:
		// vector of pointer to the peers who have it
		std::vector<const peer_connection *> m_peers;
		
};

//todo move constructor
class piece_manager{
	public:
		piece_manager() = default;
		piece_manager(std::vector<char>& piece_hashes);
		piece_manager& operator=(piece_manager && other);
		//updates all pieces for this peer.
		void update(const peer_connection * conn, const aux::bitfield& bf);
		std::vector<piece> rarest_first();
	private:
		std::vector<piece> m_pieces;
		std::priority_queue<int, std::vector<int>, std::greater<int>> m_heap;
};

// whenever a bitfiled arrives at a peer, the peer will notify the pice manager
// about the pieces the remote peer has.       
