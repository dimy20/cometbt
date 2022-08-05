#include "block.h"
block::block(){
	m_block_buffer = buffer(BLOCK_SIZE);
};

block::block(block && other){
	m_block_buffer = std::move(other.m_block_buffer);
};

block::block(const block& other){
	m_block_buffer = other.m_block_buffer;
};

block& block::operator=(block && other){
	if(this == &other) return *this;
	m_block_buffer = std::move(other.m_block_buffer);
	return *this;
};

block& block::operator=(const block& other){
	m_block_buffer = other.m_block_buffer;
	return *this;
}
