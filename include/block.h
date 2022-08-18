#pragma once
#include "buffer.h"
#define BLOCK_SIZE 1024 * 16

class block{
	public:
		block();
		block(block && other);
		block(const block& other);
		block& operator=(block && other);
		block& operator=(const block& other);

		block(char * buffer, int size, int index, int offset);
	private:
		buffer m_block_buffer;
};
