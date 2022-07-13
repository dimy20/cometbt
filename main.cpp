#include "session.h"

int main(){
	Session s("ubuntu-22.04-desktop-amd64.iso.torrent");
	s.start();
	//s.add_torrent("ubuntu-22.04-desktop-amd64.iso.torrent");
	return 0;
};

