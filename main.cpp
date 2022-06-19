#include <vector>
#include <iostream>
#include <unordered_map>

enum class becode_type{
	INT = 1,
	STRING,
	LIST,
	DICT
};

struct becode_node{
	becode_type type;
	void * val;
};

becode_type get_type(char c){
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

struct becode_node * becode_string(std::string& s, int offset, int n){
	std::string ans = "";
	for(int i = offset; i < offset + n; i++){
		ans += s[i];
	}
	struct becode_node * node = (struct becode_node *)malloc(sizeof(struct becode_node));
	if(!node) return nullptr;
	node->type = becode_type::STRING;
	node->val = (std::string *)malloc(sizeof(std::string));
	*reinterpret_cast<std::string *>(node->val) = ans;
	return node;
}

struct becode_node * becode_int(const std::string& s, int& i){
	std::string ans = "";
	while(s[i] != 'e'){
		ans += s[i];
		i++;
	}
	struct becode_node * node = (struct becode_node*)malloc(sizeof(struct becode_node));
	node->type = becode_type::INT;
	node->val = malloc(sizeof(int));
	*reinterpret_cast<int*>(node->val) = std::stoi(ans);
	return node;
}

struct becode_node * parse_list(std::string& s, int& i){
	std::vector<becode_node *> ans;
	std::string elem = "";
	int len;
	becode_type type;
	while(s[i] != 'e'){
		type = get_type(s[i]);
		switch(type){
			case becode_type::INT:
				{
					struct becode_node * node = becode_int(s, ++i);
					ans.push_back(node);
					//ans.push_back(std::to_string(*(int*)node->val));
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
					struct becode_node * node = becode_string(s, i, len);
					//ans.push_back(*reinterpret_cast<std::string*>(node->val));
					//ans.push_back(becode_string(s, i, len));
					ans.push_back(node);
					i += len - 1;
					elem = "";
					break;
				}
		}
		i++;
	}
	struct becode_node * list_node = (struct becode_node *)malloc(sizeof(struct becode_node));
	list_node->type = becode_type::LIST;
	list_node->val = (void*)malloc(sizeof(std::vector<struct becode_node*>));

	*reinterpret_cast<std::vector<struct becode_node *> *>(list_node->val) = ans;
	return list_node;
}

std::string list_to_string(std::vector<struct becode_node*>& becode_list){
	int n;
	n = becode_list.size();
	struct becode_node * node;
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

std::string node_to_string(struct becode_node * node){
	switch(node->type){
		case becode_type::LIST:
			return list_to_string(*reinterpret_cast<std::vector<struct becode_node*> *>(node->val));
		case becode_type::INT:
			return std::to_string(*reinterpret_cast<int*>(node->val));
		case becode_type::STRING:
			return *reinterpret_cast<std::string*>(node->val);
	}
}

std::string get_key(std::string& s, int& i){
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
	struct becode_node * node = becode_string(s, i, len);
	ans = *reinterpret_cast<std::string*>(node->val);
	i += len;
	free(node);
	return ans;
}

int get_str_len(std::string& s, int& i){
	std::string len_s = "";
	while(s[i] != ':'){
		len_s += s[i];
		i++;
	}
	i++;
	return std::stoi(len_s);
}
struct becode_node * parse_dict(std::string& s, int& i, int n){
	std::unordered_map<std::string, struct becode_node *> d;
	int len;
	while(i < n && s[i] != 'e'){
		std::string key;
		key = get_key(s, i);
		becode_type type = get_type(s[i]);
		switch(type){
			case becode_type::INT:
				{
					struct becode_node * node  = becode_int(s, ++i); /*skip i char*/
					d[key] = node;
					break;
				}
			case becode_type::STRING:
				{
					len = get_str_len(s, i);
					struct becode_node * node = becode_string(s, i, len);
					i += len-1;
					d[key] = node;
					break;
				}
			case becode_type::LIST:
				struct becode_node * node = parse_list(s, ++i); // skip l char
				std::vector<struct becode_node *> list_nodes = *reinterpret_cast<std::vector<struct becode_node *> *>(node->val);
				d[key] = node;
				break;

		}
		i++;
	}

	struct becode_node * dict_node = new struct becode_node;
	dict_node->type = becode_type::DICT;
	dict_node->val = new std::unordered_map<std::string, struct becode_node *>;
	*reinterpret_cast<std::unordered_map<std::string, struct becode_node *> *>(dict_node->val) = d;
	return dict_node;

}
std::string dict_to_string(std::unordered_map<std::string, struct becode_node*>& dict){
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
/*
 * integets -> ints or std::strings
 * string -> std::strings
 * dicts -> std::unordered_maps
 * lists -> std::vector<struct becode_nodes> -> 
 * std::vector<struct becode_node> 
 * becode_node = {type, content}
 *
 * */
//std::unordered_map<std::string, struct 
std::vector<std::string> parse_bencode(std::string s){
	int n;
	n = s.size();
	std::vector<std::string> ans;
	std::string elem = "";
	int len;
	std::string list = "list [";
	for(int i = 0; i < n; ++i){
		becode_type type = get_type(s[i]);
		switch(type){
			case becode_type::INT:
				{
					struct becode_node * node  = becode_int(s, ++i);
					ans.push_back(std::to_string(*reinterpret_cast<int*>(node->val)));
					elem = "";
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
					struct becode_node * node = becode_string(s, i, len);
					ans.push_back(*reinterpret_cast<std::string*>(node->val));
					i += len-1;
					elem = "";
					break;
				}
			case becode_type::LIST:
				{
					i++;
					struct becode_node * node = parse_list(s, i);
					std::vector<struct becode_node *> list_nodes = *reinterpret_cast<std::vector<struct becode_node*> *>(node->val);
				
					ans.push_back(list_to_string(list_nodes));
					list = "list [";
					break;
				}
			case becode_type::DICT:
				{
					i++;
					struct becode_node * node = parse_dict(s, i, n);
					std::unordered_map<std::string, struct becode_node*> dict = *reinterpret_cast<std::unordered_map<std::string, struct becode_node*> *>(node->val);
					ans.push_back(dict_to_string(dict));
					break;
				}
		}

	}
	return ans;
}


int main(){
	std::vector<becode_node> file;
	std::vector<std::string> vec;

	std::string val;
	while(std::cin >> val);

	for(auto x : parse_bencode(val)) std::cout << x << std::endl;

	return 0;
}
