#include "bencode.h"

int main(){
	/*
	std::string encoded;
	while(std::cin >> encoded);
*/

	/*
	std::string part1 = "d8:announce41:http://bttracker.debian.org:6969/announce7:comment35:\"Debian CD from cdimage.debian.org\"13:creation datei1573903810e";
	std::string part2 = 
"9:httpseedsl145:https://cdimage.debian.org/cdimage/release/10.2.0//srv/cdbuilder.debian.org/dst/deb-cd/weekly-builds/amd64/iso-cd/debian-10.2.0-amd64-netinst.iso145:https://cdimage.debian.org/cdimage/archive/10.2.0//srv/cdbuilder.debian.org/dst/deb-cd/weekly-builds/amd64/iso-cd/debian-10.2.0-amd64-netinst.isoe4:infod6:lengthi351272960e4:name31:debian-10.2.0-amd64-netinst.iso12:piece lengthi262144eee";

	std::string test = part1 + part2;
	std::cout << test[442] << std::endl;
	std::cout << test.size() << std::endl;
	Bencode::Decoder decoder(test);
	std::shared_ptr<struct Bencode::Bnode> data = decoder.decode();
	std::cout << decoder.to_string() << std::endl;
	*/

	std::string encoded;
	while(std::cin >> encoded);
	Bencode::Decoder decoder(encoded);
	std::shared_ptr<struct Bencode::Bnode> data = decoder.decode();
	std::cout << decoder.to_string();

	/*
	Bencode::dict_t document = std::get<Bencode::dict_t>(data->m_val);

	std::string num = std::get<std::string>(document["num"]->m_val);
	std::string hi = std::get<std::string>(document["hi"]->m_val);
	std::cout << num << std::endl;
	std::cout << hi << std::endl;
	*/


/*

"4:infod6:lengthi351272960e4:name31:debian-10.2.0-amd64-netinst.iso12:piece lengthi262144e6:pieces26800:�����PS�^�� (binary blob of the hashes of each piece)ee
"
*/
	return 0;
};
