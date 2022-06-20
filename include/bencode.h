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


class Bencode{
	
};
