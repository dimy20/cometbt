#pragma once
#include <string>
#include <cstddef>
#include <climits>
#include <assert.h>

class buffer{
	public:
		buffer();
		buffer(std::size_t size);
		buffer(std::size_t size, char * src, std::size_t src_size); 
		buffer(buffer && other);
		buffer& operator=(buffer && other);
		buffer& operator=(const buffer&  other);

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
		~buffer();
	private:
		std::size_t m_size;
		char * m_begin;
};
