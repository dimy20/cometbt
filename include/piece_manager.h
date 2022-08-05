#pragma once
#include <vector>
#include <queue>
#include "info_hash.h"
#include "bitfield.h"
#include "peer_connection.h"
#include "piece.h"

class peer_connection;
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
