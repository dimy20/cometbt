#pragma once

#include <string>
#include <sstream>
#include <openssl/sha.h>
#include <openssl/crypto.h>

class info_hash{
	public:
		info_hash(char * begin, std::size_t size);
		std::string to_string_hex();

	private:
		unsigned char m_sha1_hash[SHA_DIGEST_LENGTH];
};
