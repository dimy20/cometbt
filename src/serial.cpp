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

int aux::deserialize_int(const char * begin, const char * end){
	std::ptrdiff_t diff = end - begin;
	if(diff < 4) return -1;
	int ans;
	if(is_little_endian){
		//store as least significant byte first
		ans = static_cast<int>((unsigned char)begin[0] << 24 |
							   (unsigned char)begin[1] << 16 |
							   (unsigned char)begin[2] << 8  |
							   (unsigned char)begin[3]);
	}else{
		// store as least significat byte last (same as buff)
		ans = static_cast<int>((unsigned char)begin[3] << 24 |
							   (unsigned char)begin[2] << 16 |
							   (unsigned char)begin[1] << 8  |
							   (unsigned char)begin[0]);
	}
	return ans;
};
