#pragma once

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include <string>

class SocketTcp{
	public:
		SocketTcp();
		void connect_to(const std::string& host, const std::string& port);
		int get_fd();
	private:
		int m_fd;
};
