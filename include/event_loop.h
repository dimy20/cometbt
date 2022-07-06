#pragma once
#include <sys/epoll.h>
#include <unordered_map>
#include "tcp.h"

typedef void(* ev_cb)(SocketTcp * sock);

struct io_s{
	SocketTcp * sock;
	ev_cb cb;
};

class EventLoop{
	public:
		enum ev_type{
			READ = 1
		};
		EventLoop();
		void watch(SocketTcp * sock, ev_type ev, ev_cb cb);
		void run();
	private:
		int m_efd;
		int m_sock_count;
		std::unordered_map<int, struct io_s> m_iomap; // fd -> peer
};
