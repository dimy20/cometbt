#include "torrent.h"
#include <stdio.h>
#include <sstream>

static bool is_little_endian(){
	int num = 1;
	return (*reinterpret_cast<char *>(&num) == 1);
};

static int get_length(const char * const buff, std::size_t size){
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

static void serialize_int(std::uint8_t * begin, std::uint8_t * end, int value){
	if((end - begin + 1) < 4) return;
	begin[3] = value & 0xff;
	begin[2] = value >> 8 & 0xff;
	begin[1] = value >> 16 & 0xff;
	begin[0] = value >> 24 & 0xff;
};

static std::shared_ptr<struct interested_message> create_interested_message(){
	struct interested_message * msg = new struct interested_message;
	serialize_int(msg->length, msg->length + 3, 1);
	std::shared_ptr<struct interested_message> msg_ptr(msg);
	return msg_ptr;
};

static std::shared_ptr<struct req_message> create_request_message(int piece_index, int block_offset, int block_length){
	struct req_message * msg = new struct req_message;
	memset(msg, 0, sizeof(struct req_message));

	serialize_int(msg->length, msg->length + 3, sizeof(*msg) - 4);
	msg->id = static_cast<std::uint8_t>(message_id::REQUEST);
	serialize_int(msg->index, msg->index + 3, piece_index);
	serialize_int(msg->block_offset, msg->block_offset + 3, block_offset);
	serialize_int(msg->block_length, msg->block_length + 3, block_length);

	std::shared_ptr<struct req_message> msg_ptr(msg);
	return msg_ptr;
}
static void read_cb(SocketTcp * sock){
	Peer * peer = dynamic_cast<Peer *>(sock);
	int n;
	if(peer->wait_handshake()){
		n = peer->recv(peer->m_buff + peer->m_total, BUFF_SIZE - peer->m_total);
		peer->m_total += n;
		if(peer->m_total == 0) std::cout << "keep-alive" << std::endl;
		if(peer->m_total >= 4){
			peer->m_msg_len = get_length(peer->m_buff, peer->m_total);
			/*
			std::cout << "*****************" << std::endl;
			std::cout << "total : " << peer->m_total << std::endl;
			std::cout << "message length : " << peer->m_msg_len << std::endl;
			std::cout << "*****************" << std::endl;
			*/
			if(4 + peer->m_msg_len > peer->m_total) return;
			else{
				do_message(peer, peer->m_buff + 4, peer->m_msg_len);
				//clear buffer to receive next message
				memset(peer->m_buff, 0, peer->m_total);
				peer->m_total = 0;
			}

		}
	} else std::cout << "handshake failed " << std::endl;
};

static void die(const std::string& msg){
	std::cerr << msg << std::endl;
	exit(EXIT_FAILURE);
}

static std::string gen_peer_id(){
	std::string ans = "-PC0001-";
	for(int i = 0; i < 12; i++){
		ans += std::to_string(rand() % 10);
	};
	return ans;
};

static std::vector<char> open_file(const std::string& filename){
	std::vector<char> buff;
	int size;

	std::ifstream file(filename, std::ios::binary | std::ios::ate);
	size = file.tellg();
	file.seekg(0, std::ios::beg);
	buff.resize(size);

	file.read(buff.data(), size);
	return buff;
};

Torrent::Torrent(const std::string& filename){
	m_buff = open_file(filename);
	/*optional params*/
	m_announce_list = {};
	m_comment = "";
	m_created_by = "";
	m_creation_date = 0;
	m_encoding = "";
	m_info_private = -1;
	m_id = gen_peer_id();

	init_torrent_data();
	m_sock = SocketSSL();
};

static std::string info_file_path(Bencode::dict_t& d){
	auto list_node = std::get<Bencode::list_t>(d["path"]->m_val)[0];
	auto buff = std::get<std::vector<char>>(list_node->m_val);
	return std::string(buff.data(), buff.data() + buff.size());
};

static std::vector<info_file_t> extract_info_files(Bencode::dict_t& info){
	int length;
	std::vector<info_file_t> info_files;
	for(auto elem : std::get<Bencode::list_t>(info["files"]->m_val)){
		auto file = std::get<Bencode::dict_t>(elem->m_val);
		length = std::get<long long>(file["length"]->m_val);
		info_files.push_back({length, std::move(info_file_path(file))});
	}
	return info_files;
};

static int find_pattern(const std::vector<char>& buff, const std::vector<char>& pattern){
	int n, pattern_len;
	n = buff.size();
	pattern_len = pattern.size();
	int j = 0;
	for(int i = 0; i < n - pattern_len; i++){
		if(buff[i] == pattern[j]){
			j++;
			if(j == pattern_len){
				return i - j + 1; /*match starts at index : i-j+1*/
			}
		}else{
			i -= j;
			j = 0;
		}
	}
	return -1;
};

std::string hash_to_hex(std::vector<unsigned char> hash){
	char sha1_hex[(SHA_DIGEST_LENGTH * 2) + 1];
	for (int i = 0; i < SHA_DIGEST_LENGTH; i++){
		sprintf(sha1_hex + (i * 2),"%02x\n", hash[i]);
	}

	sha1_hex[SHA_DIGEST_LENGTH * 2] = '\0';
	std::string s = std::string(sha1_hex);

	std::stringstream ss;
	for(int i = 0; i < s.size(); i+=2){ /*Tracker requieres this */
		ss << "%" << s[i] << s[i+1];
	};

	return ss.str();
}

std::vector<unsigned char> info_hash(std::vector<char> bencode){
	std::vector<unsigned char> sha1(SHA_DIGEST_LENGTH);
	int n, start, end, info_len;
	n = bencode.size();
	start = 0;

	start = find_pattern(bencode, {'4',':','i', 'n', 'f', 'o', 'd'});
	if(start == -1) die("Error : info dictionary not found in bencode.");
	start += 6; // hash must start at info's value including 'd'

	end = n-2;
	info_len = end - start + 1;

	SHA1(reinterpret_cast<unsigned char*>(&bencode[start]), info_len, sha1.data());
	return sha1;
};

void Torrent::init_torrent_data(){
	Bencode::Decoder decoder;
	decoder.set_bencode(m_buff);

	m_info_hash = std::move(info_hash(m_buff));
	m_infohash_hex = hash_to_hex(m_info_hash);

	auto data = decoder.decode();
	auto document = std::get<Bencode::dict_t>(data->m_val);

	auto buff_tmp = std::get<std::vector<char>>(document["announce"]->m_val);
	m_announce = std::string(&buff_tmp[0], buff_tmp.data() + buff_tmp.size());

	if(document["announce-list"] != nullptr){ /*optional*/
		for(auto elem: std::get<Bencode::list_t>(document["announce-list"]->m_val)){
			auto elem_list = std::get<Bencode::list_t>(elem->m_val);
			auto announce = std::get<std::vector<char>>(elem_list[0]->m_val);
			m_announce_list.push_back(std::string(&announce[0], &announce[announce.size()]));
		};
	}
	
	if(document["comment"] != nullptr){ /*optional*/
		buff_tmp = std::get<std::vector<char>>(document["comment"]->m_val);
		m_comment = std::string(buff_tmp.data(), buff_tmp.data() + buff_tmp.size());
	}

	if(document["comment"] != nullptr){ /*optional*/
		buff_tmp = std::get<std::vector<char>>(document["created by"]->m_val);
		m_created_by = std::string(buff_tmp.data(), buff_tmp.data() + buff_tmp.size());
	};

	if(document["comment"] != nullptr){ /*optional*/
		m_creation_date = std::get<long long>(document["creation date"]->m_val);
	}

	if(document["encoding"] != nullptr){ /*optional*/
		buff_tmp = std::get<std::vector<char>>(document["encoding"]->m_val);
		m_encoding = std::string(buff_tmp.data(), buff_tmp.data() + buff_tmp.size());
	}

	/*info dict*/
	m_info = std::get<Bencode::dict_t>(document["info"]->m_val);

	buff_tmp = std::get<std::vector<char>>(m_info["name"]->m_val);
	m_info_name = std::string(buff_tmp.data(), buff_tmp.data() + buff_tmp.size());

	if(m_info["files"] != nullptr){ /*only in multiple file mode*/
		m_info_files = std::move(extract_info_files(m_info));
	}else if(m_info["length"] != nullptr){ /*single file mode*/
		m_length = std::get<long long>(m_info["length"]->m_val);
	}

	/*Common fields to both file modes*/
	if(m_info["private"] != nullptr){
		/*will remain unsuported for now*/
		m_info_private = std::get<long long>(m_info["private"]->m_val);
	}

	m_info_piecelen = std::get<long long>(m_info["piece length"]->m_val);
	m_info_pieces = std::get<std::vector<char>>(m_info["pieces"]->m_val);

};



std::string Torrent::build_request(const std::string& host){
	long long left = (m_info_pieces.size() / 20) * m_info_piecelen;

	std::stringstream ss;
	ss << "GET /announce?info_hash=" << m_infohash_hex << "&";
	ss << "peer_id=" << m_id << "&";
	ss << "uploaded=0&downloaded=0&";
	ss << "left=" << std::to_string(left) << "&";
	ss << "port=" << "443&" << "compact=1";
	ss << " HTTP/1.1\r\n";
	ss << "Host: " << host.c_str() << "\r\n\r\n";

	std::string s =	ss.str();
	return ss.str();
};

static std::string get_host(const std::string& url){
	int n, count, i;

	n = url.size(), count = 0, i = 8;
	for(i = 8; i <n & url[i] != '/'; i++){
		count++;
	};
	return url.substr(8, count);
};

static auto parse_header(const char * msg, std::size_t size){
	std::unordered_map<std::string, std::string> header;

	const char * start, * end, * msg_end;
	start = end = msg;
	msg_end = msg + size;

	while(end != msg_end && *end != ' ') end++;
	header["version"] = std::string(start, end);
	while(end != msg_end && *end == ' ') end++; // trim spaces
	start = end;
	while(end != msg_end && *end != ' ') end++;
	header["status"] = std::string(start, end);

	while(end != msg_end && *end != '\r') end++;
	end += 2; // skip \r\n


	const char * colon, * value_start;
	while(end != msg_end && *end != '\r'){
		start = end;
		while(end != msg_end && *end != '\r') end++;
		colon = (const char *)memchr(start, ':', end-start);
		if(!colon){
			std::cout << "Error : Bad response" << std::endl;
			exit(1);
		}else{
			value_start = colon + 1;
			while(value_start != end && *value_start == ' ') value_start++;
			header[std::string(start, colon)] = std::string(value_start, end);
			start = end + 2;
			end += 2;
		}

	};
	return header;
}

static const char * get_body(const char * msg, const char * msg_end){
	int index = find_pattern(std::vector<char>(msg, msg_end), {'\r', '\n', '\r', '\n'});
	if(index == -1) std::cerr << "bad request" << std::endl;
	return msg + index + 4;
}

const std::vector<Peer> Torrent::get_peers(){
	int err;

	std::string host = get_host(m_announce);
	m_sock.connect_to(host, "443");

	auto req = build_request(host); // build request for tracker
	m_sock.send(req.data(), req.size());

	char buff[BUFF_SIZE];
	int n = m_sock.recv(buff, BUFF_SIZE);

	auto headers = parse_header(buff, n);
	int content_len = std::stoi(headers["Content-Length"]);
	const char * body = get_body(buff, buff + n);


	Bencode::Decoder decoder;
	decoder.set_bencode(std::vector<char>(body, body + content_len));
	auto response = decoder.decode();
	auto document = std::get<Bencode::dict_t>(response->m_val);
	auto peers = std::get<Bencode::list_t>(document["peers"]->m_val);

	for(auto peer : peers){
		auto peer_doc = std::get<Bencode::dict_t>(peer->m_val);
		auto id = std::get<std::vector<char>>(peer_doc["peer id"]->m_val);
		auto ip = std::get<std::vector<char>>(peer_doc["ip"]->m_val);
		auto port = std::get<long long>(peer_doc["port"]->m_val);
		std::string ip_s(ip.data(), ip.size());
		m_peers.push_back(Peer(id, ip_s, std::to_string(port)));
	}

	return std::vector<Peer>(m_peers.begin(), m_peers.begin() + 1);
};

void Torrent::download_file(){
	EventLoop loop;
	auto peers = get_peers();
	int err;
	for(auto& peer : peers){
		err = peer.connect_to(peer.m_ip, peer.m_port);
		if(err != -1){
			peer.set_flags(O_NONBLOCK);
			std::cout << " ip -> " << peer.m_ip << " port-> " << peer.m_port << std::endl;
		}
	}

	// loop watch this sockets
	for(auto& peer : peers){
		loop.watch(&peer, EventLoop::ev_type::READ, read_cb);
	};

	// send handshake to all peers received by trackers
	for(auto& peer : peers){
		assert(peer.m_sock_state == SocketTcp::socket_state::CONNECTED);
		std::cout << "sending handshake to : " << peer.m_ip << std::endl;
		peer.send_handshake(m_info_hash, m_id);
	}

	loop.run();
	return;
};

void Peer::send_handshake(const std::vector<unsigned char>& info_hash, const std::string& id){
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


Peer::Peer(std::vector<char> id, const std::string& ip, const std::string& port) : SocketTcp() {
	m_id = std::move(id);
	m_ip = std::move(ip);
	m_port = std::move(port);
};

bool Peer::wait_handshake(){
	/* check if handshake has be done already*/
	if(((m_state & p_state::HANDSHAKE_WAIT) == 0) && (m_state & p_state::HANDSHAKE_DONE))
		return true;
	else if(m_state & p_state::HANDSHAKE_FAIL) return false;

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
		m_state = Peer::p_state::HANDSHAKE_DONE;
		return true;
	}else{
		close();
		m_state = Peer::p_state::HANDSHAKE_FAIL;
		return false;
		// what about epoll??
	}
	// bitfield here?
};
