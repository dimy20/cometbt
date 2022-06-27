#include "bencode.h"

static bool digit_overflows(int curr_value, int next_digit){
	return ((curr_value > (INT_MAX) / 10) ||
			(curr_value == (INT_MAX / 10) &&
			 next_digit > (INT_MAX % 10)));
};

static void die(const std::string& msg){
	std::cerr << "Error : " << msg << std::endl;
	exit(EXIT_FAILURE);
}

char Bencode::Decoder::peek(){
	if(m_index == m_bencode.size()) return EOF;
	return m_bencode[m_index];
};

void Bencode::Decoder::step(int step_count){
	if(m_index + step_count <= m_bencode.size()){
		m_index += step_count;
	}
};

Bencode::Decoder::Decoder(){
	m_index = 0;
	m_node = nullptr;
};

Bencode::Decoder::Decoder(std::vector<char> bencode){
	m_bencode = bencode;
	m_index = 0;
}

void Bencode::Decoder::set_bencode(const std::vector<char>& bencode){
	m_bencode = bencode;
	m_index = 0;
};

std::shared_ptr<struct Bencode::Bnode> Bencode::Decoder::decode_string(int n){
	std::vector<char> ans(n);
	int j = 0;
	if(m_index + n <= m_bencode.size()){
		for(int i = m_index; i < m_index + n; i++){
			ans[j] = m_bencode[i];
			j++;
		}

		std::shared_ptr<struct Bnode> node(new struct Bnode);
		node->m_val = std::move(ans);
		return node;

	} else throw std::invalid_argument("Unexpected end of string.");
}

std::shared_ptr<struct Bencode::Bnode> Bencode::Decoder::decode_int(){
	int ans, char_count;

	ans = char_count = 0;
	while(m_index < m_bencode.size() && m_bencode[m_index] != token_type::END_TOKEN){
		if(!std::isdigit(m_bencode[m_index]))
			throw std::invalid_argument("Invalid syntax (Non numeric value)");

		if(digit_overflows(ans, m_bencode[m_index]))
			throw std::invalid_argument("Integer overflow.");

		ans = ans * 10 + (m_bencode[m_index] - '0');
		char_count++;
		m_index++;
	}

	if(m_index == m_bencode.size() || char_count == 0)
			throw std::invalid_argument("Invalid syntax (Empty integer)");

	std::shared_ptr<struct Bnode> node(new struct Bnode);
	node->m_val = ans;

	return node;
}

std::shared_ptr<struct Bencode::Bnode> Bencode::Decoder::decode_list(){
	list_t ans;
	int len;

	while(m_index < m_bencode.size() && m_bencode[m_index] != Bencode::token_type::END_TOKEN){
		std::shared_ptr<struct Bnode> node = decode();
		ans.push_back(node);
	};

	if(m_index >= m_bencode.size())
		throw std::invalid_argument("Invalid syntax");

	std::shared_ptr<struct Bnode> list_node(new struct Bnode);
	list_node->m_val = std::move(ans);
	return list_node;
}

std::shared_ptr<struct Bencode::Bnode> Bencode::Decoder::decode_dict(){
	dict_t d;
	int len;
	std::shared_ptr<struct Bnode> key_node;
	while(m_index < m_bencode.size() && m_bencode[m_index] != token_type::END_TOKEN){
		key_node = decode();

		if(!std::holds_alternative<std::vector<char>>(key_node->m_val))
			throw std::invalid_argument("Invalid syntax : Key must be of string type");

		auto buff = std::get<std::vector<char>>(key_node->m_val);
		auto key = std::string(&buff[0], buff.size());
		d[key] = decode();
	}

	if(m_index >= m_bencode.size())
			throw std::invalid_argument("Invalid syntax : Unexpected end of string");

	std::shared_ptr<struct Bnode> dict_node(new struct Bnode);
	dict_node->m_val = std::move(d);
	return dict_node;
}

std::shared_ptr<struct Bencode::Bnode> Bencode::Decoder::decode(){
	int n;
	n = m_bencode.size();
	char token = peek();

	if(token == EOF || m_index >= n) return NULL;

	switch(token){
		case Bencode::token_type::INT_TOKEN:
			{
				step(1);
				m_node  = decode_int();
				step(1);
				return m_node;
			}
		case Bencode::token_type::LIST_TOKEN:
			{
				step(1); // skip token
				m_node = decode_list();
				step(1); // skip end token
				return m_node;
			}
		case Bencode::token_type::DICT_TOKEN:
			{
				step(1);
				m_node = decode_dict();
				step(1);
				return m_node;
			}
		case '0'...'9':
			{
				int len = get_str_len();
				m_node = decode_string(len);
				step(len);
				return m_node;
			}
		default:
			throw std::invalid_argument("Invalid syntax");
	}

	return m_node;
}

int Bencode::Decoder::get_str_len(){
	int ans = 0;
	while(m_bencode[m_index] != Bencode::token_type::SDEL_TOKEN){
		if(m_index == m_bencode.size())
			throw std::invalid_argument("Unexpected end of string.");

		if(!std::isdigit(m_bencode[m_index]) && (m_bencode[m_index] != ':'))
			throw std::invalid_argument("Expected \":\"");

		if(digit_overflows(ans, m_bencode[m_index]))
			throw std::invalid_argument("String too big.");

		ans = ans * 10 + (m_bencode[m_index] - '0');
		m_index++;
	}

	m_index++;
	return ans;
}

std::string Bencode::Decoder::dict_to_string(dict_t& dict){
	std::string ans = "dict { ";
	std::string value = "";
	int i = 0;
	int n = dict.size();
	for(auto& [key, node] : dict){
		ans += key + " : ";
		ans += node_to_string(node);
		if(i < n-1) ans += ", ";
		i++;
	}
	ans += " }";
	return ans;
}

std::string Bencode::Decoder::list_to_string(list_t& becode_list){
	int n;
	n = becode_list.size();
	std::string ans = "list [";

	for(int i = 0; i < n; i++){
		ans += node_to_string(becode_list[i]);
		if(i < n-1) ans += ", ";
	}
	ans += "]";
	return ans;
}

std::string Bencode::Decoder::node_to_string(std::shared_ptr<struct Bnode> node){
	int i = node->m_val.index();
	std::string ans;
	if(std::holds_alternative<int>(node->m_val))
		return std::to_string(std::get<int>(node->m_val));
	else if(std::holds_alternative<std::vector<char>>(node->m_val)){
		auto buff = std::get<std::vector<char>>(node->m_val);
		return std::string(&buff[0], buff.size());
	}else if (std::holds_alternative<list_t>(node->m_val)){
		return list_to_string(std::get<list_t>(node->m_val));
	}else if (std::holds_alternative<dict_t>(node->m_val)){
		return dict_to_string(std::get<dict_t>(node->m_val));
	}
	return ans;
};

std::string Bencode::Decoder::to_string(){
	return node_to_string(m_node) + "\n";
}
