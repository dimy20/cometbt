#include "torrent.h"

int main(){
	Torrent client("ubuntu-22.04-desktop-amd64.iso.torrent");
	auto peers = client.get_peers();
	for(auto peer : peers){
		std::cout << "ip -> " << peer.ip << " port-> " << peer.port << std::endl;
	}
	return 0;
};
