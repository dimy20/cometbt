#pragma once
#include <string>
#include <cstddef>
#include <climits>
#include <assert.h>

class Buffer{
	public:
		Buffer(std::size_t size);
		Buffer(std::size_t size, char * src, std::size_t src_size); 
		Buffer(Buffer && other);
		Buffer& operator=(Buffer && other);
		char * data();
		char const * data() const;
		std::size_t size();
		bool empty();

		char const& operator[](std::size_t index) const;
		char& operator[](std::size_t index);

		char * begin();
		char const * begin() const;
		char * end();
		char const * end() const;
		~Buffer();
	private:
		std::size_t m_size;
		char * m_begin;
};
