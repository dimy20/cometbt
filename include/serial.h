#pragma once
#include <cstddef>
#include <memory>
namespace aux{
	// checks local machine's endianness
	bool is_little_endian();
	// serialize an int into a buffer in network ordering
	void serialize_int(std::uint8_t * begin, std::uint8_t * end, int value);
};
