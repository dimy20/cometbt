#include "torrent.h"
#include "event_loop.h"

static bool is_little_endian(){
	int num = 1;
	return (*reinterpret_cast<char *>(&num) == 1);
};

static int get_length(char * buff, std::size_t size){
	if(size < 4) return -1;
	int ans;
	if(is_little_endian()){
		ans = static_cast<int>((unsigned char)buff[0] << 24 | 
							   (unsigned char)buff[1] << 16 | 
							   (unsigned char)buff[2] << 8  | 
							   (unsigned char)buff[3]);
	}else{
		ans = static_cast<int>((unsigned char)buff[3] << 24 | 
							   (unsigned char)buff[2] << 16 | 
							   (unsigned char)buff[1] << 8  | 
							   (unsigned char)buff[0]);
	}
	return ans;
};

void read_cb(SocketTcp * sock){
	std::cout << "data!! " << std::endl;
	Peer * peer = dynamic_cast<Peer *>(sock);
	int n;
	if(peer->wait_handshake()){
		if(peer->m_state & Peer::p_state::HANDSHAKE_DONE){
			char buff[BUFF_SIZE];
			int len;

			memset(buff, 0, BUFF_SIZE);
			n = peer->recv(buff, BUFF_SIZE);
			std::cout << n << std::endl;
			len = get_length(buff, n);
			//std::cout << "length : " << len << std::endl;
		}
	} 
};

int main(){
	Torrent client("ubuntu-22.04-desktop-amd64.iso.torrent");
	auto peers = client.get_peers();

	EventLoop loop;
	int err;
	for(auto& peer : peers){
		err = peer.connect_to(peer.m_ip, peer.m_port);
		if(err != -1){
			peer.set_flags(O_NONBLOCK);
			std::cout << " ip -> " << peer.m_ip << " port-> " << peer.m_port << std::endl;
		}
	}


	for(auto& peer : peers){
		loop.watch(&peer, EventLoop::ev_type::READ, read_cb);
	};

	for(auto& peer : peers){
		if((peer.m_state & SocketTcp::state_op::CLOSED) == 0){
			std::cout << "sending handshake to : " << peer.m_ip << std::endl;
			peer.send_handshake(client.m_info_hash, client.m_id);
		}
	}

	loop.run();

	return 0;
};

