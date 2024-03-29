#pragma once
#include <vector>
#include <iostream>
#include <unordered_map>
#include <variant>
#include <memory>
#include <climits>

namespace bencode{

	struct Bnode;

	using list_t = std::vector<std::shared_ptr<struct Bnode>>;
	using dict_t = std::unordered_map<std::string, std::shared_ptr<struct Bnode>>;

	struct Bnode{
		std::variant<long long, std::vector<char>, list_t, dict_t> m_val;
	};

	enum token_type {
		END_TOKEN  = 'e',
		INT_TOKEN  = 'i',
		LIST_TOKEN = 'l',
		DICT_TOKEN = 'd',
		SDEL_TOKEN = ':'
	};

	class decoder{
		public:
			decoder(std::vector<char> bencode);
			decoder();
			std::shared_ptr<struct Bnode> decode(void);
			std::string to_string(void);
			void set_bencode(const std::vector<char>& bencode);

		private:
			std::vector<char> m_bencode;
			int m_index;
			std::shared_ptr<struct Bnode> m_node;
			
		private:
			std::shared_ptr<struct Bnode> decode_string(long long n);
			std::shared_ptr<struct Bnode> decode_int();
			std::shared_ptr<struct Bnode> decode_list();
			std::shared_ptr<struct Bnode> decode_dict();

			long long get_str_len();

			std::string node_to_string(std::shared_ptr<struct Bnode> node);
			std::string dict_to_string(dict_t& dict);
			std::string list_to_string(list_t& list);

			char peek();
			void step(int step_count);

	};
};


