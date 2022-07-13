#pragma once
#include <cstddef>
#include <iostream>
#include <string.h>
#include <unordered_map>
namespace http_parser{
	using header_t = std::unordered_map<std::string, std::string>;
	header_t parse_header(const char * msg, std::size_t size);
};
