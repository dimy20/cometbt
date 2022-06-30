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

SocketTcp::SocketTcp(){
	m_fd = 0;
};

void SocketTcp::connect_to(const std::string& host, const std::string& port){
	struct addrinfo hints, * servinfo, * p;
    memset(&hints,0,sizeof(hints));

    hints.ai_family = AF_UNSPEC; /*ipv4 or ipv6*/
    hints.ai_flags = AI_PASSIVE; /* wildcard addr if no node is specified*/
    hints.ai_socktype = SOCK_STREAM;
    int ret, yes = 1;

    ret = getaddrinfo(host.c_str(), port.c_str(), &hints, &servinfo);

	if(ret == -1) perror("getaddrinfo");

	p = servinfo;

	while(p != nullptr){
		m_fd = new_socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if(m_fd == -1) continue;
		
		ret = connect(m_fd, p->ai_addr, p->ai_addrlen); /*takes too long*/
		if(ret == -1) perror("connect");

		break;
		p = p->ai_next;
	};

    freeaddrinfo(servinfo);
}

int SocketTcp::get_fd(){ return m_fd; };
