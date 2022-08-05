#include "buffer.h"
#include <string.h>

buffer::buffer(){
	m_begin = nullptr;
	m_size = 0;
};

buffer::buffer(std::size_t size){
	assert(size < INT_MAX);
	if(size == 0) return;
	m_begin = static_cast<char*>(std::malloc(sizeof(char) * size));
	if(!m_begin) throw(std::bad_alloc());
	memset(m_begin, 0, size);
	m_size = size;
};

buffer::buffer(std::size_t size, char * src, std::size_t src_size){
	assert(src_size <= size);
	m_begin = static_cast<char *>(std::malloc(size));
	m_size = size;
	if(src != nullptr){
		std::size_t len = std::min(src_size, size);
		std::copy(src, src + len, m_begin);
	}
}

buffer::buffer(buffer && other): m_begin(other.m_begin), m_size(other.m_size){
	other.m_begin = nullptr;
	other.m_size = 0;
}

buffer& buffer::operator=(buffer && other){
	if(&other == this) return *this;
	std::free(m_begin);
	m_begin = other.m_begin;
	m_size = other.m_size;
	other.m_begin = nullptr;
	other.m_size = 0;
	return *this;
};

buffer& buffer::operator=(const buffer& other){
	if(other.m_begin != nullptr && other.m_size > 0){
		m_begin = static_cast<char*>(std::malloc(other.m_size));
		if(!m_begin){
			throw(std::bad_alloc());
		}
		memcpy(m_begin, other.m_begin, other.m_size);
		m_size = other.m_size;
		return *this;
	};
	m_begin = nullptr;
	m_size = 0;
	return *this;
};

buffer::~buffer(){
	if(m_begin != nullptr)
		std::free(m_begin);
}

char * buffer::data(){
	return m_begin;
}

/*
char const * buffer::data() const{
	return m_begin;
}
*/

std::size_t buffer::size(){
	return m_size;
}

bool buffer::empty(){
	return m_size == 0;
}

char const& buffer::operator[](std::size_t index) const{
	assert(index <= m_size - 1);
	return m_begin[index];
}

char& buffer::operator[](std::size_t index){
	assert(index <= m_size - 1);
	return m_begin[index];

}

char const * buffer::begin() const {
	return m_begin;
}

char * buffer::begin(){
	return m_begin;
}

char * buffer::end(){
	return m_begin + m_size;
}
char const * buffer::end() const{
	return m_begin + m_size;
}
