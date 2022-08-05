#pragma once

#include <string>
#include <sstream>
#include <string.h>
#include <openssl/sha.h>
#include <openssl/crypto.h>

//todo -> add copy constructor
namespace aux{
	class info_hash{
		public:
			info_hash() = default;
			info_hash(char * begin, std::size_t size);
			info_hash(info_hash && other);
			info_hash(const info_hash & other);
			~info_hash();
			info_hash& operator=(const info_hash& other);
			info_hash& operator=(info_hash && other);

			
			std::string hex_str() const;
			bool operator==(const info_hash& other) const;
			bool operator!=(const info_hash& other) const;

			// populates buffer with pre-computed sha1 hash, if buffer contains
			// a sha1 digest already it will be overwritten.
			void set(char * begin, std::size_t size);
			// get raw sha1 digest
			const unsigned char * get() const { return m_sha1_hash; };

		private:
			unsigned char * m_sha1_hash = nullptr;
	};
}

