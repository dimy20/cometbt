#include "torrent.h"

int main(){
	Torrent client("ubuntu-22.04-desktop-amd64.iso.torrent");
	client.download_file();
	return 0;
};

