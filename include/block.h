#pragma once
#include "buffer.h"
#define BLOCK_SIZE 1024 * 16

class block{
	friend class piece;
	public:
		block();
		block(block && other);
		block(const block& other);
		block& operator=(block && other);
		block& operator=(const block& other);

		block(char * buffer, int size, int index, int offset);
		int index(){ return m_index; };
		int offset(){ return m_piece_offset; };

	private:
		// block of data, subset of the piece specified by index
		buffer m_buffer;
		// piece index
		int m_index;
		// byte offset within the piece
		int m_piece_offset;
};
