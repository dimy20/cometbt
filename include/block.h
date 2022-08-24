#pragma once
#include "buffer.h"
#define BLOCK_SIZE 1024 * 16

typedef struct block_s{
	// block data
	char * m_data = nullptr;
	// piece index
	int m_index;
	// byte offset within the piece
	int m_offset;
	// block size
	int m_size;
}block_t;

