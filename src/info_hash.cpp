#include "info_hash.h"

info_hash::info_hash(char * begin, std::size_t size){
	SHA1(reinterpret_cast<unsigned char*>(begin), size, m_sha1_hash);
};


std::string info_hash::to_string_hex(){
	char sha1_hex[(SHA_DIGEST_LENGTH * 2) + 1];
	for (int i = 0; i < SHA_DIGEST_LENGTH; i++){
		sprintf(sha1_hex + (i * 2),"%02x\n", m_sha1_hash[i]);
	}

	sha1_hex[SHA_DIGEST_LENGTH * 2] = '\0';
	std::string s = std::string(sha1_hex);

	std::stringstream ss;
	for(int i = 0; i < s.size(); i+=2){ /*Tracker requieres this */
		ss << "%" << s[i] << s[i+1];
	};

	return ss.str();
};
