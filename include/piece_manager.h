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
		piece_manager(long long piece_len, std::vector<char>& piece_hashes);
		piece_manager& operator=(piece_manager && other);
		std::vector<piece> m_pieces;
		std::queue<int>& get_work_queue() { return m_work_queue; };
		piece& get_piece(int index);

	private:
		std::queue<int> m_work_queue;
};
