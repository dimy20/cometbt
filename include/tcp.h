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

class SocketTcp{
	public:
		SocketTcp();
		int connect_to(const std::string& host, const std::string& port);
		int recv(char * buff, int size);
		int send(char * buff, int size);
	private:
		int m_fd;
};

