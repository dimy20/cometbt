#include "buffer.h"
#include <string.h>

Buffer::Buffer(){
	m_begin = nullptr;
	m_size = 0;
};

Buffer::Buffer(std::size_t size){
	assert(size < INT_MAX);
	if(size == 0) return;
	m_begin = static_cast<char*>(std::malloc(sizeof(char) * size));
	if(!m_begin) throw(std::bad_alloc());
	memset(m_begin, 0, size);
	m_size = size;
};

Buffer::Buffer(std::size_t size, char * src, std::size_t src_size){
	assert(src_size <= size);
	m_begin = static_cast<char *>(std::malloc(size));
	m_size = size;
	if(src != nullptr){
		std::size_t len = std::min(src_size, size);
		std::copy(src, src + len, m_begin);
	}
}

Buffer::Buffer(Buffer && other): m_begin(other.m_begin), m_size(other.m_size){
	other.m_begin = nullptr;
	other.m_size = 0;
}

Buffer& Buffer::operator=(Buffer && other){
	if(&other == this) return *this;
	std::free(m_begin);
	m_begin = other.m_begin;
	m_size = other.m_size;
	other.m_begin = nullptr;
	other.m_size = 0;
	return *this;
};

Buffer::~Buffer(){
	if(m_begin != nullptr)
		std::free(m_begin);
}

char * Buffer::data(){
	return m_begin;
}

/*
char const * Buffer::data() const{
	return m_begin;
}
*/

std::size_t Buffer::size(){
	return m_size;
}

bool Buffer::empty(){
	return m_size == 0;
}

char const& Buffer::operator[](std::size_t index) const{
	assert(index <= m_size - 1);
	return m_begin[index];
}

char& Buffer::operator[](std::size_t index){
	assert(index <= m_size - 1);
	return m_begin[index];

}

char const * Buffer::begin() const {
	return m_begin;
}

char * Buffer::begin(){
	return m_begin;
}

char * Buffer::end(){
	return m_begin + m_size;
}
char const * Buffer::end() const{
	return m_begin + m_size;
}
