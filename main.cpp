#include "torrent.h"

int main(){
	Torrent client("ubuntu-22.04-desktop-amd64.iso.torrent");
	std::string response = client.get_peers();
	std::cout << response << std::endl;
	return 0;
};
