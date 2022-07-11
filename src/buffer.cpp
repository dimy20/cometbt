#include "buffer.h"

Buffer::Buffer(std::size_t size){
	assert(size < INT_MAX);
	if(size == 0) return;
	m_begin = static_cast<char*>(std::malloc(sizeof(char) * size));
	if(!m_begin) throw(std::bad_alloc());
	m_size = size;
};

Buffer::Buffer(std::size_t size, char * src, std::size_t src_size){
	assert(src_size <= size);
	if(src != nullptr){
		std::size_t len = std::min(src_size, size);
		std::copy(src, src + len, m_begin);
	}
}
// move constructor
Buffer::Buffer(Buffer && other): m_begin(other.m_begin), m_size(other.m_size){
	other.m_begin = nullptr;
	other.m_size = 0;
}

Buffer::~Buffer(){
	std::free(m_begin);
}

char * Buffer::data(){
	return m_begin;
}

char const * Buffer::data() const{
	return m_begin;
}

std::size_t Buffer::size(){
	return m_size;
}
