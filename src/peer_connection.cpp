#include "peer_connection.h"

int peer_connection::get_length(const char * const buff, std::size_t size){
	return aux::deserialize_int(buff, buff + size);
};

peer_connection::peer_connection(const struct peer_info_s & p_info, EventLoop * loop) 
	:PeerConnectionCore(p_info, loop)
{
	m_total = 0;
	m_choked = true;
}

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

static std::shared_ptr<struct interested_message> create_interested_message(){
	struct interested_message * msg = new struct interested_message;
	aux::serialize_int(msg->length, msg->length + 3, 1);
	std::shared_ptr<struct interested_message> msg_ptr(msg);
	return msg_ptr;
};

void peer_connection::do_message(){
	auto [msg_buff, payload_len] = m_recv_buffer.get();
	std::cout << "processing message - payload : " << payload_len <<  std::endl;
	message_id msg_id  = static_cast<message_id>(*msg_buff);
	switch((msg_id)){
		case message_id::BITFIELD:
			{
				msg_buff++;
				std::cout << "bitfield !! " << std::endl;
				auto[recv_buffer, size] = m_recv_buffer.get();
				handle_bitfield(recv_buffer, size - 1);
				break;
			}
		case message_id::UNCHOKE:
			{
				m_choked = false;
				std::cout << "unchoke!" << std::endl;

				int piece_index = 0; // test first piece
				if(m_bitfield.has_piece(piece_index)){
					std::cout << "asking for piece " << piece_index << std::endl;
					auto msg = create_request_message(piece_index, 0, BLOCK_LENGTH);
					send(reinterpret_cast<char *>(msg.get()), sizeof(*msg.get()));
				}else{
					std::cout << "peer doesnt have piece with index : " << piece_index;
					std::cout << std::endl;
				};

				break;
			}
		case message_id::CHOKE:
			m_choked = true;
			std::cout << "choke" << std::endl;
			break;
		case message_id::PIECE:
			std::cout << "received block" << std::endl;
			break;
		default:
			std::cout << "msg id : " << *msg_buff << std::endl;
	}
};

void peer_connection::on_receive(int passed_bytes){

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

		aux::info_hash received_hash;
		received_hash.set(chunk, 20);

		if(m_peer_info.m_info_hash != received_hash){
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
			do_message();
			m_state = p_state::NOT_IMPLEMENTED_YET;
		}
	}

	if(m_state == p_state::NOT_IMPLEMENTED_YET){
		exit(1);
	}

};

void peer_connection::handle_bitfield(char * begin, std::size_t size){
	m_bitfield = std::move(aux::bitfield(begin, size - 1));
	if(m_choked){
		std::cout << "sending interested " << std::endl;
		auto msg = create_interested_message();
		int n = send(reinterpret_cast<char *>(msg.get()), 5);
	}
};
