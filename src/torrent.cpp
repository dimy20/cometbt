#include "torrent.h"
#include <stdio.h>
#include <sstream>

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

torrent::torrent(){
	m_announce_list = {};
	m_comment = "";
	m_created_by = "";
	m_creation_date = 0;
	m_encoding = "";
	m_info_private = -1;
	m_id = gen_peer_id();
	m_sock = SocketSSL();
};

torrent::torrent(const std::string& filename){
	/*optional params*/
	m_announce_list = {};
	m_comment = "";
	m_created_by = "";
	m_creation_date = 0;
	m_encoding = "";
	m_info_private = -1;
	m_id = gen_peer_id();
	m_sock = SocketSSL();
};

static std::string info_file_path(bencode::dict_t& d){
	auto list_node = std::get<bencode::list_t>(d["path"]->m_val)[0];
	auto buff = std::get<std::vector<char>>(list_node->m_val);
	return std::string(buff.data(), buff.data() + buff.size());
};

static std::vector<info_file_t> extract_info_files(bencode::dict_t& info){
	int length;
	std::vector<info_file_t> info_files;
	for(auto elem : std::get<bencode::list_t>(info["files"]->m_val)){
		auto file = std::get<bencode::dict_t>(elem->m_val);
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

static int find_info_dict(const std::vector<char>& bencode, int& size){
	int n, start, end, info_len;
	n = bencode.size();
	start = 0;

	start = find_pattern(bencode, {'4',':','i', 'n', 'f', 'o', 'd'});
	if(start == -1) die("Error : info dictionary not found in bencode.");
	start += 6; // hash must start at info's value including 'd'

	end = n-2;
	info_len = end - start + 1;

	size = info_len;
	return start;
};

void torrent::init_torrent_data(){
	bencode::decoder b_decoder;
	b_decoder.set_bencode(m_buff);

	int info_size, info_begin;
	info_begin = find_info_dict(m_buff, info_size);
	m_info_hash = std::move(aux::info_hash(m_buff.data() + info_begin, info_size));

	auto data = b_decoder.decode();
	auto document = std::get<bencode::dict_t>(data->m_val);

	auto buff_tmp = std::get<std::vector<char>>(document["announce"]->m_val);
	m_announce = std::string(&buff_tmp[0], buff_tmp.data() + buff_tmp.size());

	if(document["announce-list"] != nullptr){ /*optional*/
		for(auto elem: std::get<bencode::list_t>(document["announce-list"]->m_val)){
			auto elem_list = std::get<bencode::list_t>(elem->m_val);
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
	m_info = std::get<bencode::dict_t>(document["info"]->m_val);

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

static std::string get_host(const std::string& url){
	int n, count, i;

	n = url.size(), count = 0, i = 8;
	for(i = 8; i <n & url[i] != '/'; i++){
		count++;
	};
	return url.substr(8, count);
};

void torrent::setup_peerinfo(){
	int err;

	//TODO remove this from here to a lower layer
	std::string host = get_host(m_announce);
	m_sock.connect_to(host, "443");

	http_parser::query_params_s params;

	long long left = (m_info_pieces.size() / 20) * m_info_piecelen;

	params["info_hash"] = m_info_hash.hex_str();
	params["peer_id"] = m_id;
	params["uploaded"] = "0";
	params["downloaded"] = "0";
	params["left"] = std::to_string(left);
	params["port"] = "443";
	params["compact"] = "1";
	


	auto req = http_parser::build_request(host, "/announce", params);
	m_sock.send(req.data(), req.size());

	char buff[BUFF_SIZE];
	int n = m_sock.recv(buff, BUFF_SIZE);

	auto headers = http_parser::parse_header(buff, n);
	int content_len = std::stoi(headers["Content-Length"]);
	const char * body = http_parser::get_body(buff, buff + n);



	bencode::decoder b_decoder;
	b_decoder.set_bencode(std::vector<char>(body, body + content_len));
	auto response = b_decoder.decode();
	auto document = std::get<bencode::dict_t>(response->m_val);
	auto peers = std::get<bencode::list_t>(document["peers"]->m_val);

	for(auto peer : peers){
		auto peer_doc = std::get<bencode::dict_t>(peer->m_val);
		auto id = std::get<std::vector<char>>(peer_doc["peer id"]->m_val);
		auto ip = std::get<std::vector<char>>(peer_doc["ip"]->m_val);
		auto port = std::get<long long>(peer_doc["port"]->m_val);
		std::string ip_s(ip.data(), ip.size());

		
		struct peer_info_s peer_info;
		peer_info.m_remote_id = std::move(id);
		peer_info.m_remote_ip = std::move(ip_s);
		peer_info.m_remote_port = std::to_string(port);
		peer_info.m_id = m_id;
		peer_info.m_info_hash = m_info_hash;

		m_peers_info.push_back(std::move(peer_info));

	}
};

void torrent::set(std::vector<char> && torrent){
	m_buff = std::move(torrent);
};
