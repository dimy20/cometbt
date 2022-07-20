#pragma once
#include <cassert>
#include <cstring>
#include <stdlib.h>
#include <new>

namespace aux{
	class bitfield{
		public:
			bitfield() : m_bitfield(nullptr), m_size(0) {};
			bitfield(const char * begin, std::size_t size);
			bitfield& operator=(bitfield && other);
			bitfield(bitfield && other);
			~bitfield();
			bool has_piece(int index);
		private:
			char * m_bitfield = nullptr;
			std::size_t m_size = 0;
	};
};

