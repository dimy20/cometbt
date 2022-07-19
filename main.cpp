#include "session.h"

int main(){
	Session s("ubuntu-22.04-desktop-amd64.iso.torrent");
	s.start();
	return 0;
};

