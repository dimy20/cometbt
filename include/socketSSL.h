#pragma once

#include <iostream>

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <assert.h>

#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "log.h"

class SocketSSL{
	public:
		SocketSSL();
		~SocketSSL();
		SocketSSL& operator=(SocketSSL && other);
		operator bool();
		void connect_to(const std::string& host, const std::string& port);
		int recv(char * buff, int size);
		int send(char * buff, int size);
	private:
		void init_openssl();
	private:
		uint8_t m_flags;
		int m_fd;
		SSL_CTX * m_ctx;
		SSL * m_ssl = nullptr;

};

