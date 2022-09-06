#include "peer_connection.h"
#include "serial.h"

void on_close(uv_handle_t* handle) { std::cout << "closed" << std::endl; };

typedef struct stream_data_s{
	peer_connection_core * core_owner;
	char * curr_recv_chunk;
	size_t chunk_size;
}stream_data_t;

static void alloc_cb(uv_handle_t* handle, size_t size, uv_buf_t* buf) {
	stream_data_t * data = (stream_data_t *)handle->data;
	buf->base = data->curr_recv_chunk;
	buf->len = data->chunk_size;
}

void on_read(uv_stream_t* tcp, ssize_t nread, const uv_buf_t* buf)
{
    if(nread >= 0) {
		stream_data_t * data = (stream_data_t *)tcp->data;
		assert(data != nullptr);
		peer_connection_core * core_peer = data->core_owner;
		assert(data != nullptr);
		core_peer->on_receive_internal(nread);
    }
    else {
		std::cout << "Failed to read : ";
		std::cout << uv_strerror(nread) << std::endl;
		uv_close((uv_handle_t*)tcp, on_close);
    }
}

void handshake_write_cb(uv_write_t *req, int status) {
	std::cout << "handshake callback " << std::endl;
    if (status) {
        fprintf(stderr, "Write error %s\n", uv_err_name(status));
    }
	uv_buf_t * buff = (uv_buf_t *)req->data;
	assert(buff != nullptr);

	free(buff->base);
	free(buff);
	free(req);
}

peer_connection_core::peer_connection_core(const struct peer_info_s& peer_info){
	m_peer_info = peer_info;
};

peer_connection_core::peer_connection_core(peer_connection_core && other){
	m_recv_buffer = std::move(other.m_recv_buffer);
	m_peer_info = other.m_peer_info;
	m_loop = other.m_loop;
	m_socket = other.m_socket;
};

// part of this can be moved up to peer_connection
void peer_connection_core::send_handshake(const aux::info_hash& info_hash, const std::string& id){
	assert(info_hash.get() != nullptr);

	struct handshake_s hs;
	hs.proto_id_size = 19;
	memset(&hs.protocol_id, 0, sizeof(hs) - 1);
	memcpy(hs.protocol_id, PROTOCOL_ID, PROTOCOL_ID_LENGTH);
	memcpy(hs.info_hash, info_hash.get(), INFO_HASH_LENGTH);
	memcpy(hs.peer_id, id.c_str(), PEER_ID_LENGTH);

	uv_buf_t * buff = (uv_buf_t *)malloc(sizeof(uv_buf_t));
	COMET_ASSERT_ALLOC(buff);

	buff->base = static_cast<char *>(malloc(HANDSHAKE_SIZE));
	COMET_ASSERT_ALLOC(buff->base);

	buff->len = HANDSHAKE_SIZE;

	memcpy(buff->base, &hs, HANDSHAKE_SIZE);
	uv_write_t * req = (uv_write_t *)malloc(sizeof(uv_write_t));
	COMET_ASSERT_ALLOC(req);

	// point to buff and free it on callback
	req->data = buff;

	uv_write(req, m_socket, buff, 1, handshake_write_cb);

	// get ready to received
	stream_data_t * data = (stream_data_t *)m_socket->data;
	assert(data != nullptr);

	auto [span, span_size] = m_recv_buffer.reserve(1024 * 16);

	data->curr_recv_chunk = span;
	data->chunk_size = span_size;

	uv_read_start(m_socket, alloc_cb, on_read);

	m_state = peer_connection_core::p_state::READ_PROTOCOL_ID;
	// start waiting for comming protocol identifier
	m_recv_buffer.reset(20);
};

void on_connect(uv_connect_t *req, int status){
	if(status < 0){
		COMET_LOG_ERROR(std::cerr, "on_connect: " << uv_strerror(status))
		return;
	}

	// get handle
	uv_stream_t * socket = (uv_stream_t *)req->handle;

	stream_data_t * data = (stream_data_t *)req->data;
	peer_connection_core * core = data->core_owner;

	// take stream data from the connection request and assign it to the handle.
	socket->data = data;
	req->data = nullptr;
	core->m_socket = socket;

	std::cout << core->m_peer_info.m_remote_ip << std::endl;

	core->send_handshake(core->m_peer_info.m_info_hash, core->m_peer_info.m_id);
	free(req);
};

void peer_connection_core::start(uv_loop_t * loop){
	assert(loop != nullptr);
	int err;
	m_loop = loop;
	auto [span, span_size] = m_recv_buffer.reserve(1024 * 16);

	uv_tcp_t * socket = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
	COMET_ASSERT_ALLOC(socket);

	uv_tcp_init(loop, socket);

	uv_connect_t* connect = (uv_connect_t*)malloc(sizeof(uv_connect_t));
	COMET_ASSERT_ALLOC(connect);
	// pin
	stream_data_t * data = (stream_data_t *)malloc(sizeof(stream_data_t));
	COMET_ASSERT_ALLOC(data);

	data->core_owner = this;
	data->curr_recv_chunk = nullptr;
	data->chunk_size = 0;

	connect->data = data;

	int ip_ver = aux::ip_version(m_peer_info.m_remote_ip.c_str());

	if(ip_ver == 6){
		int port = std::stoi(m_peer_info.m_remote_port);
		struct sockaddr_in6 dest;
		uv_ip6_addr(m_peer_info.m_remote_ip.c_str(), port, &dest);
		uv_tcp_connect(connect, socket, (const struct sockaddr*)&dest, on_connect);

	}else if (ip_ver == 4){
		int port = std::stoi(m_peer_info.m_remote_port);
		struct sockaddr_in dest;
		uv_ip4_addr(m_peer_info.m_remote_ip.c_str(), port, &dest);
		uv_tcp_connect(connect, socket, (const struct sockaddr*)&dest, on_connect);
	}else{
		free(connect);
		free(data);
		COMET_LOG_ERROR(std::cerr, "Invalid ip address");
	}
};

void peer_connection_core::on_receive_internal(int received_bytes){
	assert(received_bytes <= m_recv_buffer.max_receive());
			
	// likely to be more data to read, grow buffer
	bool grow = m_recv_buffer.max_receive() == received_bytes;
	//acount for received bytes
	m_recv_buffer.received(received_bytes);

	int total_bytes = received_bytes;
	int passed_bytes;
	do{
		passed_bytes = m_recv_buffer.advance_pos(total_bytes);
		on_receive(passed_bytes);
		total_bytes -= passed_bytes;
	}while(total_bytes > 0 && passed_bytes > 0);

	if(m_disconnect){
		uv_close((uv_handle_t*)m_socket, on_close);
		return;
	};

	setup_receive();
};

void peer_connection_core::setup_receive(){
	// clean the processed chunks
	m_recv_buffer.clean();

	int max_receive = m_recv_buffer.max_receive();

	if(max_receive == 0){
		m_recv_buffer.grow();
	}
	if(!max_receive) m_recv_buffer.grow();

	max_receive = m_recv_buffer.max_receive();
	auto [chunk, size] = m_recv_buffer.reserve(max_receive);

	// setup uv_read
	stream_data_t * data = (stream_data_t *)m_socket->data;
	assert(data != nullptr);

	data->curr_recv_chunk = chunk;
	data->chunk_size = size;

	uv_read_start(m_socket, alloc_cb, on_read);

	assert(m_recv_buffer.pos_at_end());

};

peer_connection_core::~peer_connection_core(){
	if(m_socket != nullptr){
		free(m_socket);
	}
}
