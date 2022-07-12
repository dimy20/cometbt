#include "peer_connection.h"

bool PeerConnection::has_piece(int peer_index){
	int byte_offset, bit_offset;
	byte_offset = peer_index / 8;
	bit_offset = peer_index % 8;
	return ((*(m_bitfield + byte_offset)) >> bit_offset) & 1;
};

bool PeerConnection::wait_handshake(){
	/* check if handshake has be done already*/
	if(m_state == p_state::HANDSHAKE_DONE) return true;
	else if(m_state == p_state::HANDSHAKE_WAIT){
		char buff[HANDSHAKE_SIZE];
		bool info_hash_match, peer_id_match;
		info_hash_match = peer_id_match = false;
		int n;

		memset(buff, 0, HANDSHAKE_SIZE);
		n = recv(buff, HANDSHAKE_SIZE);
		std::cout << "peer " << m_ip << " sent " << n << " bytes." << std::endl;
		//wait for handshake response
		struct handshake_s * hs_reply;
		hs_reply = reinterpret_cast<struct handshake_s *>(buff);

		if(memcmp(hs_reply->info_hash, m_info_hash.data(), INFO_HASH_LENGTH) == 0)
			info_hash_match = true;
		if(memcmp(hs_reply->peer_id, m_id.data(), PEER_ID_LENGTH) == 0)
			peer_id_match = true;

		if(info_hash_match && peer_id_match){
			m_state = PeerConnection::p_state::HANDSHAKE_DONE;
			return true;
		}else{
			close();
			m_state = PeerConnection::p_state::HANDSHAKE_FAIL;
			return false;
			// what about epoll??
		}
	// bitfield here?
	}else return false;

};

PeerConnection::PeerConnection(std::vector<char> id, const std::string& ip, const std::string& port) : SocketTcp() {
	m_id = std::move(id);
	m_ip = std::move(ip);
	m_port = std::move(port);
	m_total = 0;
	memset(m_buff, 0, BUFF_SIZE);
	m_choked = true;
};

void PeerConnection::send_handshake(const std::vector<unsigned char>& info_hash, const std::string& id){
	assert(get_fd() != -1);

	struct handshake_s hs;
	hs.proto_id_size = 19;
	memset(&hs.protocol_id, 0, sizeof(hs) - 1);
	memcpy(hs.protocol_id, PROTOCOL_ID, PROTOCOL_ID_LENGTH);
	memcpy(hs.info_hash, info_hash.data(), INFO_HASH_LENGTH);
	memcpy(hs.peer_id, id.c_str(), PEER_ID_LENGTH);

	send(reinterpret_cast<char *>(&hs), HANDSHAKE_SIZE);
	m_info_hash = info_hash; // copy to verify later when handshake response comes
	m_state = p_state::HANDSHAKE_WAIT;
};
