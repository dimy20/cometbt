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
