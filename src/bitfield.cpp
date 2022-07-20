#include "bitfield.h"
#include <iostream>
aux::bitfield::bitfield(const char * begin, std::size_t size){
	assert(size > 0);
	m_bitfield = reinterpret_cast<char*>(std::malloc(size));
	if(!m_bitfield) throw std::bad_alloc();
	memcpy(m_bitfield, begin, size);
	m_size = size;
};

aux::bitfield::~bitfield(){
	free(m_bitfield);
};

bool aux::bitfield::has_piece(int piece_index){
	int byte_offset, bit_offset;
	byte_offset = piece_index / 8;
	bit_offset = piece_index % 8;
	return ((*(m_bitfield + byte_offset)) >> bit_offset) & 1;
};

aux::bitfield::bitfield(bitfield && other){ *this = std::move(other); };

aux::bitfield& aux::bitfield::operator=(bitfield && other){
	std::cout << "called" << std::endl;
	m_bitfield = other.m_bitfield;
	m_size = other.m_size;
	other.m_bitfield = nullptr;
	other.m_size = 0;
	return *this;
};
