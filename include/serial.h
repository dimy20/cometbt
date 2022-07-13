#pragma once
#include <cstddef>
#include <memory>
namespace aux{
	// checks local machine's endianness
	bool is_little_endian();
	// serialize an int into a buffer in network ordering
	void serialize_int(std::uint8_t * begin, std::uint8_t * end, int value);
	// deserialize int in buffer stored in big-endian bye ordering to local int.
	// deserialize_int expects the integer to be stored in the buffer in big-endian 
	// format
	int deserialize_int(const char * begin, const char * end);
};
