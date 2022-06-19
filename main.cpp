#include <vector>
#include <iostream>

enum class becode_type{
	INT = 1,
	STRING,
	LIST,
	DICT
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
std::string becode_string(std::string& s, int offset, int n){
	std::string ans = "";
	for(int i = offset; i < offset + n; i++){
		ans += s[i];
	}
	return ans;
}

int becode_int(const std::string& s, int& i){
	std::string ans = "";
	while(s[i] != 'e'){
		ans += s[i];
		i++;
	}
	return std::stoi(ans);
}

std::vector<std::string> parse_list(std::string& s, int& i){
	std::vector<std::string> ans;
	std::string elem = "";
	int len;
	while(s[i] != 'e'){
		if(s[i] == 'i')
			ans.push_back(std::to_string(becode_int(s, ++i)));
		else if (std::isdigit(s[i]) != 0){
			while(s[i] != ':'){
				elem += s[i];
				i++;
			}
			i++;
			len = std::stoi(elem);
			ans.push_back(becode_string(s, i, len));
			i += len - 1;
			elem = "";
		}
		i++;
	}
	return ans;
}

std::string get_key(std::string& s, int& i){
	int len;
	std::string elem = "";
	std::string ans;
	if(std::isdigit(s[i])){
		while(s[i] != ':'){
			elem += s[i];
			i++;
		}
		i++;
		len = std::stoi(elem);
	}
	ans = becode_string(s, i, len);
	i += len;
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
				ans.push_back(std::to_string(becode_int(s, ++i)));
				elem = "";
				break;
			case becode_type::STRING:
				while(s[i] != ':'){
					elem += s[i];
					i++;
				}
				i++;
				len = std::stoi(elem);
				ans.push_back(becode_string(s, i, len));
				i += len-1;
				elem = "";
				break;
			case becode_type::LIST:
				i++;
				for(auto e : parse_list(s, i)){
					list += e + ", ";
				}
				list += "]";
				ans.push_back(list);
				list = "list [";
				break;
			case becode_type::DICT:
				i++;
				while(i < n && s[i] != 'e'){
					std::string key, value;
					key = get_key(s, i);
					becode_type type = get_type(s[i]);
					std::cout << i << std::endl;
					switch(type){
						case becode_type::INT:
							i++; /*skip i char*/
							value = std::to_string(becode_int(s, i));
							break;
						case becode_type::STRING:
							len = get_str_len(s, i);
							value = becode_string(s, i, len);
							i += len-1;
							break;
						case becode_type::LIST:
							i++;
							list = "list [";
							for(auto e : parse_list(s, i)){
								list += e + ", ";
							}
							value = list + "]";
							list = "list [";
							break;

					}
					std::string entry = "";
					entry += "{" + key + ":" + value + "}";
					ans.push_back("dict " + entry);
					i++;
				}
				break;
		}

	}
	return ans;
}


int main(){
	std::vector<std::string> vec;

	std::string val;
	while(std::cin >> val);

	//parse_bencode(val);
	for(auto x : parse_bencode(val)) std::cout << x << std::endl;

	return 0;
}
