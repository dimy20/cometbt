#pragma once

#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <assert.h>
#include <fcntl.h>

class socket_tcp{
	public:
		socket_tcp();
		socket_tcp(socket_tcp && other) = default;
		int connect_to(const std::string& host, const std::string& port);
		int recv(char * buff, int size);
		int send(char * buff, int size, int& err);
		void close();
		int set_flags(int op);
		int get_fd() const;
		int read_available();
		virtual ~socket_tcp() = default;
	public:
		enum class socket_state{
			CONNECTED = 1,
			CLOSED = 2
		};
		socket_state m_sock_state;
	private:
		int m_fd;

};

