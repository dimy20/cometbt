#include "serial.h"

bool aux::is_little_endian(){
	int num = 1;
	return (*reinterpret_cast<char *>(&num) == 1);
}

void aux::serialize_int(std::uint8_t * begin, std::uint8_t * end, int value){
	if((end - begin + 1) < 4) return;
	begin[3] = value & 0xff;
	begin[2] = value >> 8 & 0xff;
	begin[1] = value >> 16 & 0xff;
	begin[0] = value >> 24 & 0xff;
}
