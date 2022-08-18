#include "block.h"
block::block(){
	m_buffer = buffer(BLOCK_SIZE);
};

block::block(block && other){
	m_buffer = std::move(other.m_buffer);
};

block::block(const block& other){
	m_buffer = other.m_buffer;
};

block& block::operator=(block && other){
	if(this == &other) return *this;
	m_buffer = std::move(other.m_buffer);
	return *this;
};

block& block::operator=(const block& other){
	m_buffer = other.m_buffer;
	return *this;
}

block::block(char * buff, int size, int index, int offset){
	m_buffer = buffer(size, buff, size);
	m_index = index;
	m_piece_offset = offset;
};
