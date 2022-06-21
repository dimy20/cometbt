#include "bencode.h"

int main(){
	std::string val;
	while(std::cin >> val);

	Bencode bencode(val);
	bencode.decode();
	std::cout << bencode.to_string();
	return 0;
}
