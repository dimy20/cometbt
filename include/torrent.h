#pragma once

#include <vector>
#include <string>
#include <fstream>
#include "bencode.h"

typedef struct info_file_s info_file_t;

struct info_file_s{
	int length;
	std::string path;
};

class Torrent{


	public:
		Torrent(const std::string& filename);
		void getData();
		const std::string& get_announce();
		const std::vector<std::string>& get_announce_list();
		const std::vector<info_file_t>& get_info_files();
	private:
		std::vector<char> m_buff;
		std::string m_announce;
		std::vector<std::string> m_announce_list;
		std::string m_comment;
		std::string m_created_by;
		int m_creation_date;
		std::string m_encoding;

		Bencode::dict_t m_info;
		std::vector<info_file_t> m_info_files;
		std::string m_info_name;
		int m_info_piecelen;
		std::vector<char> m_info_pieces; /*sha1 of each piece*/
};


