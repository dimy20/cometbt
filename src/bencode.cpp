#include "bencode.h"

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
}

struct bencode_node * Bencode::bencode_string(std::string& s, int offset, int n){
	std::string ans = "";
	for(int i = offset; i < offset + n; i++){
		ans += s[i];
	}
	struct bencode_node * node = new struct bencode_node;
	node->type = becode_type::STRING;
	node->val = new std::string;
	*reinterpret_cast<std::string *>(node->val) = ans;
	return node;
}

struct bencode_node * Bencode::bencode_int(const std::string& s, int& i){
	std::string ans = "";
	while(s[i] != 'e'){
		ans += s[i];
		i++;
	}
	struct bencode_node * node = new struct bencode_node;
	node->type = becode_type::INT;
	node->val = new int;
	*reinterpret_cast<int*>(node->val) = std::stoi(ans);
	return node;
}

struct bencode_node * Bencode::bencode_list(std::string& s, int& i){
	std::vector<struct bencode_node*> ans;
	std::string elem = "";
	int len;
	becode_type type;
	while(s[i] != 'e'){
		type = get_type(s[i]);
		switch(type){
			case becode_type::INT:
				{
					struct bencode_node * node = bencode_int(s, ++i);
					ans.push_back(node);
					break;
				}
			case becode_type::STRING:
				{
					while(s[i] != ':'){
						elem += s[i];
						i++;
					}
					i++;
					len = std::stoi(elem);
					struct bencode_node * node = bencode_string(s, i, len);
					ans.push_back(node);
					i += len - 1;
					elem = "";
					break;
				}
		}
		i++;
	}
	struct bencode_node * list_node = new struct bencode_node;
	list_node->type = becode_type::LIST;
	list_node->val = new std::vector<struct bencode_node*>;

	*reinterpret_cast<std::vector<struct bencode_node *> *>(list_node->val) = ans;
	return list_node;
}

struct bencode_node * Bencode::bencode_dict(std::string& s, int& i, int n){
	std::unordered_map<std::string, struct bencode_node *> d;
	int len;
	while(i < n && s[i] != 'e'){
		std::string key;
		key = get_key(s, i);
		becode_type type = get_type(s[i]);
		switch(type){
			case becode_type::INT:
				{
					struct bencode_node * node  = bencode_int(s, ++i); /*skip i char*/
					d[key] = node;
					break;
				}
			case becode_type::STRING:
				{
					len = get_str_len(s, i);
					struct bencode_node * node = bencode_string(s, i, len);
					i += len-1;
					d[key] = node;
					break;
				}
			case becode_type::LIST:
				struct bencode_node * node = bencode_list(s, ++i); // skip l char
				std::vector<struct bencode_node *> list_nodes = *reinterpret_cast<std::vector<struct bencode_node *> *>(node->val);
				d[key] = node;
				break;

		}
		i++;
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

std::vector<struct bencode_node *>& Bencode::parse(){
	int n;
	n = m_bencode.size();
	std::string elem = "";
	int len;
	std::vector<struct bencode_node *> ans;
	for(int i = 0; i < n; ++i){
		becode_type type = get_type(m_bencode[i]);
		switch(type){
			case becode_type::INT:
				{
					struct bencode_node * node  = bencode_int(m_bencode, ++i);
					ans.push_back(node);
					elem = "";
					break;
				}
			case becode_type::STRING:
				{
					len = get_str_len(m_bencode, i);
					struct bencode_node * node = bencode_string(m_bencode, i, len);
					ans.push_back(node);
					i += len-1;
					elem = "";
					break;
				}
			case becode_type::LIST:
				{
					i++;
					struct bencode_node * node = bencode_list(m_bencode, i);
					std::vector<struct bencode_node *> list_nodes = *reinterpret_cast<std::vector<struct bencode_node*> *>(node->val);
					ans.push_back(node);
					break;
				}
			case becode_type::DICT:
				{
					i++;
					struct bencode_node * node = bencode_dict(m_bencode, i, n);
					ans.push_back(node);
					break;
				}
		}

	}
	m_nodes = ans;
	return m_nodes;
}


std::string Bencode::get_key(std::string& s, int& i){
	int len;
	std::string elem = "";
	std::string ans = "";
	if(std::isdigit(s[i])){
		while(s[i] != ':'){
			elem += s[i];
			i++;
		}
		i++;
		len = std::stoi(elem);
	}
	struct bencode_node * node = bencode_string(s, i, len);
	ans = *reinterpret_cast<std::string*>(node->val);
	i += len;
	free(node);
	return ans;
}

int Bencode::get_str_len(std::string& s, int& i){
	std::string len_s = "";
	while(s[i] != ':'){
		len_s += s[i];
		i++;
	}
	i++;
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
