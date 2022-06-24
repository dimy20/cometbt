#pragma once
#include <vector>
#include <iostream>
#include <unordered_map>
#include <variant>
#include <memory>

namespace Bencode{

	struct Bnode;

	using list_t = std::vector<std::shared_ptr<struct Bnode>>;
	using dict_t = std::unordered_map<std::string, std::shared_ptr<struct Bnode>>;

	struct Bnode{
		std::variant<int, std::string, list_t, dict_t> m_val;
	};

	enum token_type {
		END_TOKEN  = 'e',
		INT_TOKEN  = 'i',
		LIST_TOKEN = 'l',
		DICT_TOKEN = 'd',
		SDEL_TOKEN = ':'
	};

	class Decoder{
		public:
			Decoder(std::string bencode_s);
			std::shared_ptr<struct Bnode> decode(void);
			std::string to_string(void);

		private:
			std::string m_bencode;
			int m_index;
			std::shared_ptr<struct Bnode> m_node;
			
		private:
			std::shared_ptr<struct Bnode> decode_string(int n);
			std::shared_ptr<struct Bnode> decode_int();
			std::shared_ptr<struct Bnode> decode_list();
			std::shared_ptr<struct Bnode> decode_dict();

			int get_str_len();

			std::string node_to_string(std::shared_ptr<struct Bnode> node);
			std::string dict_to_string(dict_t& dict);
			std::string list_to_string(list_t& list);

			char peek();
			void step(int step_count);

	};
};


