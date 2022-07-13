#include "http_parser.h"
http_parser::header_t http_parser::parse_header(const char * msg, std::size_t size){
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

std::string http_parser::build_request(const std::string& host,
									   const std::string& path,
									   struct query_params_s& query){

	std::stringstream ss;
	ss << "GET " << path << "?";
	ss << query.to_string();
	ss << " HTTP/1.1\r\n";
	ss << "Host: " << host.c_str() << "\r\n\r\n";

	return ss.str();
}

std::string& http_parser::query_params_s::operator[](std::string key){
	m_insert_order.push_back(key);
	return m_values[key];
};

std::string http_parser::query_params_s::to_string(){
	std::stringstream ss;
	for(auto key : m_insert_order){
		ss << key << "=" << m_values[key] << "&";
	}
	return ss.str();
};
