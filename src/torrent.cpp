#include "torrent.h"
#include <stdio.h>
#include <sstream>

static void die(const std::string& msg){
	std::cerr << msg << std::endl;
	exit(EXIT_FAILURE);
}

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


	init_torrent_data();
	init_openssl();
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

std::string info_hash(std::vector<char> bencode){
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

	/*sha1 hex representation*/
	char sha1_hex[(SHA_DIGEST_LENGTH * 2) + 1];
	for (int i = 0; i < SHA_DIGEST_LENGTH; i++){
		sprintf(sha1_hex + (i * 2),"%02x\n", sha1[i]);
	}
	sha1_hex[SHA_DIGEST_LENGTH * 2] = '\0';

	std::string s = std::string(sha1_hex);

	std::stringstream ss;
	for(int i = 0; i < s.size(); i+=2){ /*Tracker requieres this */
		ss << "%" << s[i] << s[i+1];
	};

	return ss.str();
};

void Torrent::init_torrent_data(){
	Bencode::Decoder decoder;
	decoder.set_bencode(m_buff);

	m_infohash_hex = std::move(info_hash(m_buff));

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

void Torrent::init_openssl(){
	OpenSSL_add_ssl_algorithms();
	SSL_load_error_strings();
	m_ctx = SSL_CTX_new(TLS_client_method());
};



static std::string gen_peer_id(){
	std::string ans = "-PC0001-";
	for(int i = 0; i < 12; i++){
		ans += std::to_string(rand() % 10);
	};
	return ans;
};

std::string Torrent::build_request(const std::string& host){
	auto peer_id = gen_peer_id();
	long long left = (m_info_pieces.size() / 20) * m_info_piecelen;

	std::stringstream ss;
	ss << "GET /announce?info_hash=" << m_infohash_hex << "&";
	ss << "peer_id=" << peer_id << "&";
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

int new_socket(int family, int socktype, int protocol){
	int fd, ret;
	fd = socket(family, socktype, protocol);

	if(fd < 0){
		perror("socket");
		exit(1);
	};

	return fd;
};

int connect_to(const char * host, const char * port){
	struct addrinfo hints, * servinfo, * p;
    memset(&hints,0,sizeof(hints));

    hints.ai_family = AF_UNSPEC; /*ipv4 or ipv6*/
    hints.ai_flags = AI_PASSIVE; /* wildcard addr if no node is specified*/
    hints.ai_socktype = SOCK_STREAM;
    int ret, yes = 1;

    ret = getaddrinfo(host, port, &hints, &servinfo);

	if(ret == -1) perror("getaddrinfo");

	p = servinfo;

	int fd;
	while(p != nullptr){
		fd = new_socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if(fd == -1) continue;
		
		ret = connect(fd, p->ai_addr, p->ai_addrlen); /*takes too long*/
		if(ret == -1) perror("connect");

		break;
		p = p->ai_next;
	};


    freeaddrinfo(servinfo);
	return fd;
};

std::string Torrent::get_peers(){
	int fd, err;

	std::string host = get_host(m_announce);
	fd = connect_to(host.c_str(), "443");

	m_ssl = SSL_new(m_ctx);
	if(!m_ssl) std::cerr << "failed to create SSL session" << std::endl;
	SSL_set_fd(m_ssl, fd);
	err = SSL_connect(m_ssl);
	if(!err) std::cerr << "Failed to initiate negotiaion" << std::endl;

	auto req = build_request(host); // build request for tracker

	err = SSL_write(m_ssl, req.data(), req.size());

	char buff[1024*16];
	memset(buff, 0, sizeof(buff));

	int n, total;
	total = 0;
	while(1){
		n = SSL_read(m_ssl, buff + total, (1024*16) - total);
		if(n == 0) break;
		else if (n == -1) std::cerr << "Error reading" << std::endl;
		else {
			total +=n;
		}
	};

	buff[total] = '\0';
	return std::string (std::move(buff)); /*for now, response is not te be interpreted as text*/
};
