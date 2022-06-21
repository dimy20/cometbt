#include <vector>
#include <iostream>
#include <unordered_map>

enum token_type {
	END_TOKEN = 'e',
	INT_TOKEN = 'i',
	LIST_TOKEN = 'l',
	DICT_TOKEN = 'd',
};

enum class becode_type{
	INT = 1,
	STRING,
	LIST,
	DICT
};

struct bencode_node{
	becode_type type;
	void * val;
};


class Bencode{

	public:
		Bencode(std::string bencode_s);
		std::vector<struct bencode_node *>& decode(void);
		std::string to_string(void);
		std::vector<struct bencode_node *>& nodes();

	private:
		std::string m_bencode;
		int m_index;
		std::vector<struct bencode_node *> m_nodes;
	private:
		becode_type get_type(char c);
		struct bencode_node* bencode_string(int offset, int n);
		struct bencode_node * bencode_int();
		struct bencode_node * bencode_list();
		struct bencode_node * bencode_dict(int n);

		std::string get_key();
		int get_str_len();

		std::string dict_to_string(std::unordered_map<std::string, struct bencode_node*>& dict);
		std::string list_to_string(std::vector<struct bencode_node*>& becode_list);
		std::string node_to_string(struct bencode_node * node);

		char peek();
		void step();
};
