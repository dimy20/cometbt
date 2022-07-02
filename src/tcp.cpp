#include "tcp.h"


static int new_socket(int family, int socktype, int protocol){
	int fd, ret;
	fd = socket(family, socktype, protocol);

	if(fd < 0){
		perror("socket");
		exit(1);
	};

	return fd;
};

SocketSSL::SocketSSL(){
	m_fd = 0;
	m_ssl = nullptr;
	init_openssl();
};

void SocketSSL::init_openssl(){
	OpenSSL_add_ssl_algorithms();
	SSL_load_error_strings();
	m_ctx = SSL_CTX_new(TLS_client_method());
};

void SocketSSL::connect_to(const std::string& host, const std::string& port){
	struct addrinfo hints, * servinfo, * p;
    memset(&hints,0,sizeof(hints));

    hints.ai_family = AF_UNSPEC; /*ipv4 or ipv6*/
    hints.ai_flags = AI_PASSIVE; /* wildcard addr if no node is specified*/
    hints.ai_socktype = SOCK_STREAM;
    int err, ret, yes = 1;

    ret = getaddrinfo(host.c_str(), port.c_str(), &hints, &servinfo);

	if(ret == -1) perror("getaddrinfo");

	p = servinfo;

	for(p == servinfo; p != nullptr; p = p->ai_next){
		m_fd = new_socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if(m_fd == -1) continue;

		ret = connect(m_fd, p->ai_addr, p->ai_addrlen); /*takes too long*/
		if(ret == -1){
			close(m_fd);
			std::cerr << "Error : failed to connect to " << host << std::endl;
		}else break; /*connected*/
	}

	if(p == nullptr){
		std::cerr << "Error : Unable to create socket. " << std::endl;
		exit(EXIT_FAILURE);
	}

	/* Negotiate ssl session */
	assert(m_ctx != nullptr && "m_ctx is a null pointer");
	m_ssl = SSL_new(m_ctx);
	if(!m_ssl) std::cerr << "failed to create SSL session" << std::endl;
	SSL_set_fd(m_ssl, m_fd);
	err = SSL_connect(m_ssl);
	if(!err) std::cerr << "Failed to initiate negotiaion" << std::endl;

    freeaddrinfo(servinfo);
}

int SocketSSL::send(char * buff, int size){
	assert(m_ssl != nullptr && "ssl is null, session not created");
	int err, n, total;
	total = 0;
	while(1){
		n = SSL_write(m_ssl, buff + total, size - total);
		if(n > 0){
			total += n;
		}else if(n <= 0){
			err = SSL_get_error(m_ssl, n);
			if(err == SSL_ERROR_WANT_READ){
				std::cerr << "want read" << std::endl;
			}else if(err == SSL_ERROR_WANT_WRITE){
				std::cerr << "want write" << std::endl;
			}else break;
		}
	}
	return total;
}

int SocketSSL::recv(char * buff, int size){
	int err, n, total;
	total = 0;
	memset(buff, 0, size);

	while(1){
		assert(m_ssl != nullptr && "ssl is null, session not created");
		n = SSL_read(m_ssl, buff + total, size - total);

		if(n > 0){
			total += n;
		}else if( n <= 0 ){
			err = SSL_get_error(m_ssl, n);
			if(err == SSL_ERROR_WANT_READ){
				std::cerr << "want read" << std::endl;
			}else if(err == SSL_ERROR_WANT_WRITE){
				std::cerr << "want write" << std::endl;
			}else break;
		}
	};

	buff[total + 1] = '\0';
	return total;
}

