#include "bencode.h"

int main(){
	std::string encoded;
	while(std::cin >> encoded);

	Bencode::Decoder decoder(encoded);
	std::shared_ptr<struct Bencode::Bnode> data = decoder.decode();

	std::cout << decoder.to_string();
	return 0;
}
