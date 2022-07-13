#pragma once
#include <cstddef>
#include <iostream>
#include <string.h>
#include <unordered_map>
#include <map>
#include <sstream>
#include <vector>

namespace http_parser{
	using header_t = std::unordered_map<std::string, std::string>;
	header_t parse_header(const char * msg, std::size_t size);

	struct query_params_s{
		public:
			//overload oparator to keep track of order of insertion when a new value
			//is added.
			std::string& operator[](std::string key);
			// convert value in map to a query string so it can be attached to a
			// request.
			std::string to_string();
		private:
			// key->value map
			std::unordered_map<std::string, std::string> m_values;
			// maintain the order of insertion so the end string gets build
			// in the same order as it appears when the values were
			// inserted by the caller.
			std::vector<std::string> m_insert_order;

	};
};
