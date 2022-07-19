#include "peer_connection.h"
#include "serial.h"

void read_cb(SocketTcp * sock, char * buff, std::size_t received_bytes){
	PeerConnection * peer = dynamic_cast<PeerConnection *>(sock);
	peer->on_receive_internal(received_bytes);
};


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
	m_state = p_state::READ_PROTOCOL_ID;
};


void PeerConnection::start(){
	int err;
	err = connect_to(m_peer_info.m_remote_ip, m_peer_info.m_remote_port);
	if(err == -1) std::cerr << "failed to start peer connection" << std::endl;
	set_flags(O_NONBLOCK);
	send_handshake(m_peer_info.m_info_hash, m_peer_info.m_id);

	auto [span, span_size] = m_recv_buffer.reserve(128);

	m_loop->watch(this, EventLoop::ev_type::READ, read_cb);
	m_loop->async_read(this, span, span_size);

	// start waiting for comming protocol identifier
	m_recv_buffer.reset(20);
};

void PeerConnection::on_receive_data(std::size_t received_bytes){

	if(m_state == p_state::READ_PROTOCOL_ID){
		if(!m_recv_buffer.is_message_finished()) return;
		//implement a chunk_t object
		assert(m_recv_buffer.message_size() == 20);

		auto [chunk, size] = m_recv_buffer.get();
		assert(size == 20);

		int const protocol_size = chunk[0];

		if(protocol_size != PROTOCOL_ID_LENGTH ||
				memcmp(chunk + 1, PROTOCOL_ID, protocol_size)){
			std::cerr << "Error : Unknown protocol identifier." << std::endl;
			close();
			exit(1);
		};

		// protocol id good, ready to read next step of handshake
		m_state = p_state::READ_INFO_HASH;
		m_recv_buffer.reset(RESERVED_BYTES_LENGTH + INFO_HASH_LENGTH); // 28
	};

	if(m_state == p_state::READ_INFO_HASH){

		// expecting 8 bytes for extension and 20 bytes for info hash
		assert(m_recv_buffer.message_size() == 28);

		if(!m_recv_buffer.is_message_finished()) return;

		auto [chunk, size] = m_recv_buffer.get();

		assert(size == 28);

		// we dont support extensions for now, so skip these 8 bytes
		chunk += 8;

		if (memcmp(chunk, m_peer_info.m_info_hash.data(), INFO_HASH_LENGTH) != 0){
			std::cerr << "Handshake failure : Bad info hash" << std::endl;
			exit(EXIT_FAILURE);
		}

		m_state = p_state::READ_PEER_ID;
		m_recv_buffer.reset(PEER_ID_LENGTH); // 20 bytes
	}

	if(m_state == p_state::READ_PEER_ID){
		assert(m_recv_buffer.message_size() == 20);

		if(!m_recv_buffer.is_message_finished()) return;

		auto [chunk, size] = m_recv_buffer.get();

		assert(size == 20);

		if(memcmp(chunk, m_peer_info.m_remote_id.data(), PEER_ID_LENGTH) != 0){
			std::cerr << "Error: failed to verify remote peer id" << std::endl;
			exit(EXIT_FAILURE);
		}else std::cout << "verified peer id, handshake complete" << std::endl;

		m_state = p_state::READ_MESSAGE_SIZE;
		m_recv_buffer.reset(4);
	}

	if(m_state == p_state::READ_MESSAGE_SIZE){
		assert(m_recv_buffer.message_size() == 4);

		if(!m_recv_buffer.is_message_finished()) return;


		auto [chunk, size] = m_recv_buffer.get();
		int message_len = get_length(chunk, size);

		if(message_len == 0){
			std::cout << "keep alive message" << std::endl;
			exit(1);
		}
		else if (message_len > 0){
			std::cout << "MESSAGE -> " << m_recv_buffer.message_size() << std::endl;
			m_recv_buffer.reset(message_len);
			std::cout << "message incoming of size : " << message_len << std::endl;
			m_state = p_state::READ_MESSAGE;
		}

	}

	if(m_state == p_state::READ_MESSAGE){
		if(!m_recv_buffer.is_message_finished()){
			std::cout << "message not finished" << std::endl;
			return;
		}else{
			std::cout << "received full message " << std::endl;
			auto [chunk, size] = m_recv_buffer.get();
			do_message(this, chunk, size);
			m_state = p_state::NOT_IMPLEMENTED_YET;
		}
	}

	if(m_state == p_state::NOT_IMPLEMENTED_YET){
		exit(1);
	}

};

};
