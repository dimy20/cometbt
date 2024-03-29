#include "tcp.h"

static void die(const char * msg){
	perror(msg);
	exit(EXIT_FAILURE);
};

static int new_socket(int family, int socktype, int protocol){
	int fd, ret;
	fd = socket(family, socktype, protocol);

	if(fd < 0){
		perror("socket");
		exit(1);
	};

	return fd;
};

socket_tcp::socket_tcp(){
	m_fd = -1;
};

int socket_tcp::set_flags(int flags){
	if(m_fd == -1) die("Bad fd");
	int m_flags, ret;

    m_flags = fcntl(m_fd, F_GETFL, 0);
	if(m_flags == -1) die("fcntl");

    m_flags |= flags;
	ret = fcntl(m_fd, F_SETFL, m_flags);

	if(ret == -1) die("fcntl");

	return m_flags;
};

int socket_tcp::connect_to(const std::string& host, const std::string& port){
	struct addrinfo hints, * servinfo, * p;
    memset(&hints,0,sizeof(hints));

    hints.ai_family = AF_UNSPEC; /*ipv4 or ipv6*/
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0;
    hints.ai_protocol = 0;
	
    int err, ret, yes = 1;

    ret = getaddrinfo(host.c_str(), port.c_str(), &hints, &servinfo);

	if(ret == -1) perror("getaddrinfo");

	p = servinfo;

	for(p == servinfo; p != nullptr; p = p->ai_next){
		m_fd = new_socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if(m_fd == -1) continue;

		ret = connect(m_fd, p->ai_addr, p->ai_addrlen); /*takes too long*/
		if(ret == -1){
			::close(m_fd);
			std::cerr << "Error : failed to connect to " << host << std::endl;
			m_sock_state = socket_state::CLOSED;
		}else break; /*connected*/
	}

	if(p == nullptr){
		std::cerr << "Error : Unable to create socket. " << std::endl;
		return -1;
	}

	m_sock_state = socket_state::CONNECTED;
	return m_fd;
};

int socket_tcp::recv(char * buff, int size){
	int total, n;
	total = 0;
	while(1){
		n = ::recv(m_fd, buff + total, size - total, 0);
		if(n > 0){
			total += n;
		}else if (n == -1){
			if(errno != EAGAIN && errno != EWOULDBLOCK) die("recv");
			else break; // no data right now
		}else break;
	}
	return total;
};

int socket_tcp::send(char * buff, int size, int& err){
	int total, n;
	total = 0, err = 0;
	while(1){
		n = ::send(m_fd, buff + total, size - total, 0);
		if(n > 0){
			total += n;
		}else if(n == -1){
			if(errno != EAGAIN && errno != EWOULDBLOCK) die("send");
			else{// out of space in send buffer, set epollout
				err = -1;
				break;
			}
		}else break;
	}
	return total;
};

int socket_tcp::get_fd() const {
	return m_fd;
}

void socket_tcp::close(){
	::close(m_fd);
}

int socket_tcp::read_available(){
	int ans;
	ioctl(m_fd, FIONREAD, &ans);
	return ans;
}
