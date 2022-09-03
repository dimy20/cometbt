#include "peer_connection.h"
#include "session.h"

struct send_params{
	int index;
	std::queue<int> * work_queue;
	piece_manager * p_manager;
	peer_connection * peer;
	uv_loop_t * loop;
};

struct piece_write_req{
	int index;
	piece_manager * p_manager;
};

void write_cb(uv_write_t *req, int status) {
    if (status) {
        fprintf(stderr, "Write error %s\n", uv_err_name(status));
    }
	free(req);
};

int peer_connection::get_length(const char * const buff, std::size_t size){
	return aux::deserialize_int(buff, buff + size);
};

peer_connection::peer_connection(const struct peer_info_s & p_info, piece_manager * pm,
		uv_async_t * async)
: peer_connection_core(p_info)
{
	m_total = 0;
	m_choked = true;
	m_piece_manager = pm;
	m_async = async;
}

peer_connection::peer_connection(peer_connection && other)
: peer_connection_core(std::move(other)){

	m_piece_manager = other.m_piece_manager;
	m_msg_len = other.m_msg_len;
	m_total = other.m_total;
	m_choked = other.m_choked;
	m_bitfield = std::move(m_bitfield);
	m_async = other.m_async;
//	memcpy(&m_sem_choked, &other.m_sem_choked, sizeof(sem_t));
};

peer_connection& peer_connection::operator=(const peer_connection& other){
	m_piece_manager = other.m_piece_manager;
	m_msg_len = other.m_msg_len;
	m_total = other.m_total;
	m_bitfield = other.m_bitfield;
	m_async = other.m_async;
	return *this;
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

static std::shared_ptr<struct interested_message> create_interested_message(){
	struct interested_message * msg = new struct interested_message;
	aux::serialize_int(msg->length, msg->length + 3, 1);
	std::shared_ptr<struct interested_message> msg_ptr(msg);
	return msg_ptr;
};

void peer_connection::do_message(){
	auto [msg_buff, payload_len] = m_recv_buffer.get();
	//std::cout << "processing message - payload : " << payload_len <<  std::endl;
	message_id msg_id  = static_cast<message_id>(*msg_buff);
	switch((msg_id)){
		case message_id::BITFIELD: handle_bitfield(); break;
		case message_id::UNCHOKE: handle_unchoke(); break;
		case message_id::CHOKE:   handle_choke();   break;
		case message_id::PIECE:   handle_piece();   break;
		default:
			{
				//std::cout << " unsuported msg id : " << *msg_buff << std::endl;
				m_state = p_state::READ_MESSAGE_SIZE;
				m_recv_buffer.reset(4);
				break;
		  };
			
	}
};

void peer_connection::handle_unchoke(){
	assert(m_async != nullptr);
	auto& work_queue = m_piece_manager->get_work_queue();

	struct send_params *params = (struct send_params*)malloc(sizeof(struct send_params));
	params->index = work_queue.front();
	params->work_queue = &work_queue;
	params->p_manager = m_piece_manager;
	params->peer = this;
	params->loop = m_loop;

	m_async->data = (void*)(params);
	uv_async_send(m_async);

	work_queue.pop();

	m_choked = false;
	m_state = p_state::READ_MESSAGE_SIZE;
	m_recv_buffer.reset(4);
};

int peer_connection::fetch_piece(int index, std::queue<int>& work_queue){
	// are we in the middle of dowloading a piece right now?
	piece& p = m_curr_piece == -1 ? m_piece_manager->get_piece(index):
											m_piece_manager->get_piece(m_curr_piece);

	if(m_curr_piece != -1) work_queue.push(index);

	m_curr_piece = m_curr_piece != -1 ? m_curr_piece : index;

	if(!m_bitfield.empty() && !m_bitfield.has_piece(p.index())){
		return -1;
	}
	if(p.m_backlog == MAX_BACKLOG){
		return -1;// max oustanding request at this moment
	}


	if(p.download_finished()){
		int ans;
		// schedule next piece here?
		if(p.verify_integrity()){
			std::cout << " Integrity verified, " << std::endl;
			std::cout << m_peer_info.m_remote_ip << std::endl;
			//send_have(m_curr_piece);
			ans = m_curr_piece;
		}else{
			work_queue.push(m_curr_piece);
			ans = -1;
		}
		m_curr_piece = -1;
		return ans;
	}

	int size;
	int piece_length = p.length();
	// make m_backlog oustanding requests!
	while(p.m_backlog < MAX_BACKLOG && p.m_total_requested < piece_length){
		// last block might be smaller
		if(p.m_total_requested + BLOCK_LENGTH > piece_length){
			size = piece_length - p.m_total_requested;
		}else size = BLOCK_LENGTH;

		auto msg = create_request_message(p.index(), p.m_total_requested, size);

		uv_write_t * req = (uv_write_t *)malloc(sizeof(uv_write_t));
		if(!req) std::cerr << "failed to alloacte " << std::endl;

		uv_buf_t * buf = (uv_buf_t *)malloc(sizeof(uv_buf_t));
		if(buf == nullptr) std::cerr << "FART " << std::endl;

		buf->base = (char *)msg.get();
		buf->len = sizeof(*msg.get());

		uv_write(req, m_socket, buf, 1, write_cb);

		p.m_backlog++;
		p.m_total_requested += size;
	};
	// setup lower layer to received message
	setup_receive();
	return -1;
};

void peer_connection::handle_choke(){
	m_choked = true;
	m_state = p_state::READ_MESSAGE_SIZE;
	m_recv_buffer.reset(4);
	std::cout << "choked" << std::endl;
};

void peer_connection::handle_piece(){
	auto[chunk, size] = m_recv_buffer.get();
	int block_size = size - 9;
	chunk++; // skip msg id
	int index = aux::deserialize_int(chunk, chunk + 4);
	chunk += 4;
	int offset = aux::deserialize_int(chunk, chunk + 4);
	chunk += 4;

	block_t new_block;
	new_block.m_data = chunk;
	new_block.m_size = block_size;
	new_block.m_index = index;
	new_block.m_offset = offset;

	auto& piece = m_piece_manager->get_piece(index);
	piece.incoming_block(new_block);
	m_backlog--; // allow send

	std::cout << "received block of size : " << size - 9<< std::endl;

	m_state = p_state::READ_MESSAGE_SIZE;
	m_recv_buffer.reset(4);
	return;
};

void peer_connection::on_receive(int passed_bytes){
	int message_len;

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
			m_disconnect = true;
			return;
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
			m_disconnect = true;
			return;
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
			m_disconnect = true;
			return;
		}else std::cout << "verified peer id, handshake complete" << std::endl;

		m_state = p_state::READ_MESSAGE_SIZE;
		m_recv_buffer.reset(4);
	}

	if(m_state == p_state::READ_MESSAGE_SIZE){

		assert(m_recv_buffer.message_size() == 4);

		if(!m_recv_buffer.is_message_finished()) return;

		auto [chunk, size] = m_recv_buffer.get();
		message_len = get_length(chunk, size);

		if(message_len == 0){
			//std::cout << "keep alive message" << std::endl;
			return;
		}
		else if (message_len > 0){
			m_recv_buffer.reset(message_len);
			//std::cout << "message incoming of size : " << message_len << std::endl;
			m_state = p_state::READ_MESSAGE;
		}

	}

	if(m_state == p_state::READ_MESSAGE){
		if(!m_recv_buffer.is_message_finished()){
			return;
		}else{
			std::cout << "received full message " << std::endl;
			do_message();
		}
	}
};

void peer_connection::handle_bitfield(char * begin, std::size_t size){
	m_bitfield = std::move(aux::bitfield(begin, size - 1));
void peer_connection::handle_bitfield(){
	auto [chunk, size] = m_recv_buffer.get();
	m_bitfield = std::move(aux::bitfield(chunk + 5, size - 5));
	if(m_choked){
		std::cout << "sending interested " << std::endl;
		auto msg = create_interested_message();
		uv_write_t * req = (uv_write_t *)(malloc(sizeof(uv_write_t)));
		if(!req) std::cerr << "Failed to allocate" << std::endl;

		uv_buf_t buf = {.base = (char *)msg.get(), .len = 5};
		uv_write(req, m_socket, &buf, 1, write_cb);

		m_state = p_state::READ_MESSAGE_SIZE; // wait for unchoke message
		m_recv_buffer.reset(4);

	}
};
