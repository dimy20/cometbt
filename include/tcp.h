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
#include <fcntl.h>

class SocketTcp{
	public:
		SocketTcp();
		int connect_to(const std::string& host, const std::string& port);
		int recv(char * buff, int size);
		int send(char * buff, int size);
		void close();
		int set_flags(int op);
		int get_fd() const;
	public:
		enum state_op{
			CLOSED = 1,
		};
		std::uint8_t m_state;
	private:
		int m_fd;

};

