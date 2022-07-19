#include "peer_connection.h"
#include "serial.h"

static std::shared_ptr<struct interested_message> create_interested_message(){
	struct interested_message * msg = new struct interested_message;
	aux::serialize_int(msg->length, msg->length + 3, 1);
	std::shared_ptr<struct interested_message> msg_ptr(msg);
	return msg_ptr;
};

static std::shared_ptr<struct req_message> create_request_message(int piece_index, int block_offset, int block_length){
	struct req_message * msg = new struct req_message;
	memset(msg, 0, sizeof(struct req_message));

	aux::serialize_int(msg->length, msg->length + 3, sizeof(*msg) - 4);
	msg->id = static_cast<std::uint8_t>(message_id::REQUEST);
	aux::serialize_int(msg->index, msg->index + 3, piece_index);
	aux::serialize_int(msg->block_offset, msg->block_offset + 3, block_offset);
	aux::serialize_int(msg->block_length, msg->block_length + 3, block_length);

	std::shared_ptr<struct req_message> msg_ptr(msg);
	return msg_ptr;
}

static int get_length(const char * const buff, std::size_t size){
	return aux::deserialize_int(buff, buff + size);
};

void static do_message(PeerConnection * peer, const char * msg_buff, int payload_len){
	std::cout << "processing message - payload : " << payload_len <<  std::endl;
	message_id msg_id  = static_cast<message_id>(*msg_buff);
	switch((msg_id)){
		case message_id::BITFIELD:
			{
				msg_buff++;
				peer->m_bitfield = new char[payload_len - 1];
				memcpy(peer->m_bitfield, msg_buff, payload_len -1);
				std::cout << "bitfield !! " << std::endl;

				// send intrested message
				if(peer->m_choked){
					std::cout << "sending interested " << std::endl;
					auto msg = create_interested_message();
					int n = peer->send(reinterpret_cast<char *>(msg.get()), 5);
				}
				break;
			}
		case message_id::UNCHOKE:
			{
				peer->m_choked = false;
				std::cout << "unchoke!" << std::endl;

				int piece_index = 0; // test first piece
				if(peer->has_piece(piece_index)){
					std::cout << "asking for piece " << piece_index << std::endl;
					auto msg = create_request_message(piece_index, 0, BLOCK_LENGTH);
					peer->send(reinterpret_cast<char *>(msg.get()), sizeof(*msg.get()));
				}else{
					std::cout << "peer doesnt have piece with index : " << piece_index;
					std::cout << std::endl;
				}

				break;
			}

		case message_id::CHOKE:
			peer->m_choked = true;
			std::cout << "choke" << std::endl;
			break;
		case message_id::PIECE:
			std::cout << "received block" << std::endl;
			break;
		default:
			std::cout << "msg id : " << *msg_buff << std::endl;
	}
};

bool PeerConnection::has_piece(int peer_index){
	int byte_offset, bit_offset;
	byte_offset = peer_index / 8;
	bit_offset = peer_index % 8;
	return ((*(m_bitfield + byte_offset)) >> bit_offset) & 1;
};

PeerConnection::PeerConnection(const struct peer_info_s& peer_info, EventLoop * loop){
	m_peer_info = peer_info;
	m_total = 0;
	m_choked = true;
	m_loop = loop;
};

PeerConnection::PeerConnection(PeerConnection && other) : SocketTcp(std::move(other)){
	m_recv_buffer = std::move(other.m_recv_buffer);
	m_peer_info = other.m_peer_info;
	m_loop = other.m_loop;
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
	//m_info_hash = info_hash; // copy to verify later when handshake response comes
	m_state = p_state::HANDSHAKE_WAIT;
};


void PeerConnection::start(){
	int err;
	err = connect_to(m_peer_info.m_remote_ip, m_peer_info.m_remote_port);
	if(err == -1) std::cerr << "failed to start peer connection" << std::endl;
	set_flags(O_NONBLOCK);
	send_handshake(m_peer_info.m_info_hash, m_peer_info.m_id);
};

void PeerConnection::on_receive_data(){
	std::cout << "hello" << std::endl;
	//PeerConnection * peer = dynamic_cast<PeerConnection *>(sock);
	int n;
	if(wait_handshake()){
		n = recv(m_buff + m_total, BUFF_SIZE - m_total);
		m_total += n;
		if(m_total == 0) std::cout << "keep-alive" << std::endl;
		if(m_total >= 4){
			m_msg_len = get_length(m_buff, m_total);
			/*
			std::cout << "*****************" << std::endl;
			std::cout << "total : " << peer->m_total << std::endl;
			std::cout << "message length : " << peer->m_msg_len << std::endl;
			std::cout << "*****************" << std::endl;
			*/
			if(4 + m_msg_len > m_total) return;
			else{
				do_message(this, m_buff + 4, m_msg_len);
				//clear buffer to receive next message
				memset(m_buff, 0, m_total);
				m_total = 0;
			}

		}
	} else std::cout << "handshake failed " << std::endl;
};
