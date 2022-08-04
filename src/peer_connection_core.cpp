#include "peer_connection.h"
#include "serial.h"

void read_cb(SocketTcp * sock, char * buff, std::size_t received_bytes){
	PeerConnectionCore * peer = dynamic_cast<PeerConnectionCore *>(sock);
	peer->on_receive_internal(received_bytes);
};

PeerConnectionCore::PeerConnectionCore(const struct peer_info_s& peer_info, event_loop * loop){
	m_peer_info = peer_info;
	m_loop = loop;
};

PeerConnectionCore::PeerConnectionCore(PeerConnectionCore && other) : SocketTcp(std::move(other)){
	m_recv_buffer = std::move(other.m_recv_buffer);
	m_peer_info = other.m_peer_info;
	m_loop = other.m_loop;
};

// part of this can be moved up to peer_connection
void PeerConnectionCore::send_handshake(const aux::info_hash& info_hash, const std::string& id){
	assert(get_fd() != -1);
	assert(info_hash.get() != nullptr);

	struct handshake_s hs;
	hs.proto_id_size = 19;
	memset(&hs.protocol_id, 0, sizeof(hs) - 1);
	memcpy(hs.protocol_id, PROTOCOL_ID, PROTOCOL_ID_LENGTH);
	memcpy(hs.info_hash, info_hash.get(), INFO_HASH_LENGTH);
	memcpy(hs.peer_id, id.c_str(), PEER_ID_LENGTH);

	send(reinterpret_cast<char *>(&hs), HANDSHAKE_SIZE);
	//m_info_hash = info_hash; // copy to verify later when handshake response comes
	m_state = p_state::READ_PROTOCOL_ID;
};


void PeerConnectionCore::start(){
	int err;
	err = connect_to(m_peer_info.m_remote_ip, m_peer_info.m_remote_port);
	if(err == -1) std::cerr << "failed to start peer connection" << std::endl;
	set_flags(O_NONBLOCK);
	send_handshake(m_peer_info.m_info_hash, m_peer_info.m_id);

	auto [span, span_size] = m_recv_buffer.reserve(1024 * 16);

	m_loop->watch(this, event_loop::ev_type::READ, read_cb);
	m_loop->async_read(this, span, span_size);

	// start waiting for comming protocol identifier
	m_recv_buffer.reset(20);
};

void PeerConnectionCore::on_receive_internal(int received_bytes){
	assert(received_bytes <= m_recv_buffer.max_receive());
			
	// likely to be more data to read, grow buffer
	bool grow = m_recv_buffer.max_receive() == received_bytes;
	//acount for received bytes
	m_recv_buffer.received(received_bytes);

	//another reason to do this is because epoll wont notify us again for the bytes
	//we didnt read  (edge mode)
	if(grow){
		//std::cout << "asking for more bytes " << std::endl;

		int available_bytes = read_available();

		// bytes left out
		if(available_bytes > 0){
			auto[chunk, size] = m_recv_buffer.reserve(available_bytes);

			int bytes = recv(chunk, size);
					
			assert(bytes > 0);

			m_recv_buffer.received(bytes);
			received_bytes += bytes;
		}


	}

	std::cout << "received bytes : " << received_bytes << std::endl;
	int total_bytes = received_bytes;
	int passed_bytes;
	do{
		passed_bytes = m_recv_buffer.advance_pos(total_bytes);
		on_receive(passed_bytes);
		total_bytes -= passed_bytes;
	}while(total_bytes > 0 && passed_bytes > 0);

	if(m_disconnect){
		close();
		return;
	};

	// clean the processed chunks
	m_recv_buffer.clean();


	int max_receive = m_recv_buffer.max_receive();

	if(!max_receive) m_recv_buffer.grow();

	max_receive = m_recv_buffer.max_receive();
	auto [chunk, size] = m_recv_buffer.reserve(max_receive);
	m_loop->async_read(this, chunk, size);

	assert(m_recv_buffer.pos_at_end());

};
