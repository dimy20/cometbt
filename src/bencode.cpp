#include "bencode.h"

char Bencode::peek(){
	if(m_index + 1 >= m_bencode.size()) return EOF;
	return m_bencode[m_index];
};

void Bencode::step(int step_count){
	if(m_index + step_count < m_bencode.size()){
		m_index += step_count;
	}
};

becode_type Bencode::get_type(char c){
	switch(c){
		case 'i':
			return becode_type::INT;
		case 'd':
			return becode_type::DICT;
		case 'l':
			return becode_type::LIST;
		default:
			if(std::isdigit(c) != 0) return becode_type::STRING;
	}
}

Bencode::Bencode(std::string bencode){
	m_bencode = bencode;
	m_index = 0;
}

struct bencode_node * Bencode::decode_string(int offset, int n){
	std::string ans = "";
	for(int i = offset; i < offset + n; i++){
		ans += m_bencode[i];
	}
	struct bencode_node * node = new struct bencode_node;
	node->type = becode_type::STRING;
	node->val = new std::string;
	*reinterpret_cast<std::string *>(node->val) = ans;
	return node;
}

struct bencode_node * Bencode::decode_int(){
	std::string ans = "";
	while(m_bencode[m_index] != token_type::END_TOKEN){
		ans += m_bencode[m_index];
		m_index++;
	}
	struct bencode_node * node = new struct bencode_node;
	node->type = becode_type::INT;
	node->val = new int;
	*reinterpret_cast<int*>(node->val) = std::stoi(ans);
	return node;
}

struct bencode_node * Bencode::decode_list(){
	std::vector<struct bencode_node*> ans;
	std::string elem = "";
	int len;
	becode_type type;
	while(m_bencode[m_index] != 'e'){
		type = get_type(m_bencode[m_index]);
		switch(type){
			case becode_type::INT:
				{
					step();
					struct bencode_node * node = decode_int();
					ans.push_back(node);
					break;
				}
			case becode_type::STRING:
				{
					while(m_bencode[m_index] != ':'){
						elem += m_bencode[m_index];
						m_index++;
					}
					step();
					len = std::stoi(elem);
					struct bencode_node * node = decode_string(m_index, len);
					ans.push_back(node);
					m_index += len -1;
					elem = "";
					break;
				}
		}
		m_index++;
	}
	struct bencode_node * list_node = new struct bencode_node;
	list_node->type = becode_type::LIST;
	list_node->val = new std::vector<struct bencode_node*>;

	*reinterpret_cast<std::vector<struct bencode_node *> *>(list_node->val) = ans;
	return list_node;
}

struct bencode_node * Bencode::decode_dict(int n){
	std::unordered_map<std::string, struct bencode_node *> d;
	int len;
	while(m_index < n && m_bencode[m_index] != 'e'){
		std::string key;
		key = get_key();
		becode_type type = get_type(m_bencode[m_index]);
		switch(type){
			case becode_type::INT:
				{
					step();
					struct bencode_node * node  = decode_int(); /*skip i char*/
					d[key] = node;
					break;
				}
			case becode_type::STRING:
				{
					len = get_str_len();
					struct bencode_node * node = decode_string(m_index, len);
					m_index += len -1;
					d[key] = node;
					break;
				}
			case becode_type::LIST:
				step();
				struct bencode_node * node = decode_list(); // skip l char
				std::vector<struct bencode_node *> list_nodes = *reinterpret_cast<std::vector<struct bencode_node *> *>(node->val);
				d[key] = node;
				break;

		}
		step();
	}

	struct bencode_node * dict_node = new struct bencode_node;
	dict_node->type = becode_type::DICT;
	dict_node->val = new std::unordered_map<std::string, struct bencode_node*>;
	*reinterpret_cast<std::unordered_map<std::string, struct bencode_node*> *>(dict_node->val) = d;
	return dict_node;

}

std::string Bencode::to_string(){
	int n;
	n = m_nodes.size();
	std::string ans = "";
	for(int i = 0; i < n; i++){
		ans += node_to_string(m_nodes[i]);
		ans += "\n";
	}
	return ans;
}

std::vector<struct bencode_node *>& Bencode::decode(){
	int n;
	n = m_bencode.size();
	int len;
	char token = peek();
	switch(token){
		case token_type::INT_TOKEN:
			{
				step();
				struct bencode_node * node  = decode_int();
				m_nodes.push_back(node);
				break;
			}
		case token_type::LIST_TOKEN:
			{
				step();
				struct bencode_node * node = decode_list();
				std::vector<struct bencode_node *> list_nodes = *reinterpret_cast<std::vector<struct bencode_node*> *>(node->val);
				m_nodes.push_back(node);
				break;
			}
		case token_type::DICT_TOKEN:
			{
				step();
				struct bencode_node * node = decode_dict(n);
				m_nodes.push_back(node);
				break;
			}
		default:
			if(std::isdigit(token) != 0){
				len = get_str_len();
				struct bencode_node * node = decode_string(m_index, len);
				m_nodes.push_back(node);
				m_index += len -1;
				break;
			}
	}
	return m_nodes;
}


std::string Bencode::get_key(){
	int len;
	std::string elem = "";
	std::string ans = "";
	if(std::isdigit(m_bencode[m_index])){
		while(m_bencode[m_index] != ':'){
			elem += m_bencode[m_index];
			m_index++;
		}
		m_index++;
		len = std::stoi(elem);
	}
	struct bencode_node * node = decode_string(m_index, len);
	ans = *reinterpret_cast<std::string*>(node->val);
	m_index += len;
	free(node);
	return ans;
}

int Bencode::get_str_len(){
	std::string len_s = "";
	while(m_bencode[m_index] != ':'){
		len_s += m_bencode[m_index];
		m_index++;
	}
	m_index++;
	return std::stoi(len_s);
}

std::string Bencode::dict_to_string(std::unordered_map<std::string, struct bencode_node*>& dict){
	std::string ans = "dict { ";
	std::string value = "";
	int i = 0;
	int n = dict.size();
	for(auto& [key, node] : dict){
		ans += key + " : ";
		ans += node_to_string(node);
		if(i < n-1) ans += ",";
		i++;
	}
	ans += " }";
	return ans;
}

std::string Bencode::list_to_string(std::vector<struct bencode_node*>& becode_list){
	int n;
	n = becode_list.size();
	struct bencode_node * node;
	std::string ans = "list [";

	for(int i = 0; i < n; i++){
		node = becode_list[i];
		switch(node->type){
			case becode_type::INT:
				ans += std::to_string(*reinterpret_cast<int*>(node->val));
				break;
			case becode_type::STRING:
				ans += *reinterpret_cast<std::string*>(node->val);
				break;
		} 
		if(i < n-1) ans += ", ";
	}
	ans += "]";
	return ans;
}

std::string Bencode::node_to_string(struct bencode_node * node){
	switch(node->type){
		case becode_type::LIST:
			return list_to_string(*reinterpret_cast<std::vector<struct bencode_node*> *>(node->val));
		case becode_type::INT:
			return std::to_string(*reinterpret_cast<int*>(node->val));
		case becode_type::STRING:
			return *reinterpret_cast<std::string*>(node->val);
		case becode_type::DICT:
			return dict_to_string(*reinterpret_cast<std::unordered_map<std::string, struct bencode_node*> *>(node->val));
	}
}

std::vector<struct bencode_node *>& Bencode::nodes(){
	return m_nodes;
}
