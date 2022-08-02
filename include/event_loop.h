#pragma once
#include <sys/epoll.h>
#include <unordered_map>
#include <sys/time.h>
#include "tcp.h"

typedef void(* ev_cb)(SocketTcp * sock, char * buff, std::size_t );

struct io_s{
	SocketTcp * sock;
	ev_cb cb;
	char * buff;
	std::size_t size;
};

class EventLoop{
	public:
		enum ev_type{
			READ = 1
		};
		EventLoop();
		void watch(SocketTcp * sock, ev_type ev, ev_cb cb);
		void run();
		void async_read(SocketTcp * sock, char * buff, std::size_t);
	private:
		uint64_t get_ms_time(void);

	private:
		int m_efd;
		int m_sock_count;
		std::unordered_map<int, struct io_s> m_iomap; // fd -> peer
};
