#include "info_hash.h"
#include <iostream>
aux::info_hash::info_hash(char * begin, std::size_t size){
	m_sha1_hash = (unsigned char *)std::malloc(SHA_DIGEST_LENGTH);
	if(m_sha1_hash == nullptr) throw std::bad_alloc();
	SHA1(reinterpret_cast<unsigned char*>(begin), size, m_sha1_hash);
};


std::string aux::info_hash::hex_str(){
	if(m_sha1_hash == nullptr) return "";
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

bool aux::info_hash::operator==(const info_hash& other){
	return memcmp(m_sha1_hash, other.m_sha1_hash, SHA_DIGEST_LENGTH) == 0;
};

bool aux::info_hash::operator!=(const info_hash& other){
	return memcmp(m_sha1_hash, other.m_sha1_hash, SHA_DIGEST_LENGTH) != 0;
}

aux::info_hash::~info_hash(){
	std::free(m_sha1_hash);
};

aux::info_hash& aux::info_hash::operator=(const info_hash& other){
	m_sha1_hash = reinterpret_cast<unsigned char*>(std::malloc(SHA_DIGEST_LENGTH));
	if(!m_sha1_hash) throw std::bad_alloc();
	memcpy(m_sha1_hash, other.m_sha1_hash, SHA_DIGEST_LENGTH);
	return *this;
};


aux::info_hash::info_hash(info_hash && other){
	m_sha1_hash = other.m_sha1_hash;
	other.m_sha1_hash = nullptr;
};

void aux::info_hash::set(char * begin, std::size_t size){
	if(size > SHA_DIGEST_LENGTH){
		std::cerr << "Error:invalid sha1 hash" << std::endl;
		exit(1);
	}else if(m_sha1_hash != nullptr){
		memcpy(m_sha1_hash, begin, SHA_DIGEST_LENGTH);
	}else{
		m_sha1_hash = reinterpret_cast<unsigned char *>(malloc(SHA_DIGEST_LENGTH));
		if(!m_sha1_hash) throw std::bad_alloc();
		memcpy(m_sha1_hash, begin, SHA_DIGEST_LENGTH);
	}
};
