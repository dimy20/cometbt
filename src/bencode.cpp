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
	while(m_bencode[m_index] != token_type::END_TOKEN){
		struct bencode_node * node = decode();
		if(node != NULL){
			ans.push_back(node);
		}
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
	struct bencode_node * key_node, * value_node;
	while(m_index < n && m_bencode[m_index] != token_type::END_TOKEN){
		key_node = decode();
		std::string key = *reinterpret_cast<std::string*>(key_node->val);
		delete key_node;
		value_node = decode();

		d[key] = value_node;
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

struct bencode_node * Bencode::decode(){
	int n;
	n = m_bencode.size();
	int len;
	char token = peek();

	struct bencode_node * node;

	if(token == EOF){
		return NULL;
	}

	switch(token){
		case token_type::INT_TOKEN:
			{
				step(1);
				node  = decode_int();
				m_nodes.push_back(node);
				step(1);
				return node;
			}
		case token_type::LIST_TOKEN:
			{
				step(1); // skip token
				node = decode_list();
				std::vector<struct bencode_node *> list_nodes = *reinterpret_cast<std::vector<struct bencode_node*> *>(node->val);
				m_nodes.push_back(node);
				step(1); // skip end token
				return node;
			}
		case token_type::DICT_TOKEN:
			{
				step(1);
				node = decode_dict(n);
				m_nodes.push_back(node);
				step(1);
				return node;
			}
		default:
			if(std::isdigit(token) != 0){
				len = get_str_len();
				node = decode_string(m_index, len);
				m_nodes.push_back(node);
				step(len);
				return node;
			}
	}
	return node;
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
