#pragma once
#include <sys/epoll.h>
#include <unordered_map>
#include <sys/time.h>
#include <queue>
#include <climits>
#include "tcp.h"
#include "timer.h"

typedef void(* ev_cb)(SocketTcp * sock, char * buff, std::size_t );

struct io_s{
	SocketTcp * sock;
	ev_cb cb;
	char * buff;
	std::size_t size;
};


bool compare(timer a, timer b);

class EventLoop{
	typedef std::priority_queue<timer, std::vector<timer>,decltype(&compare)> heap_timer_t;
	public:
		enum ev_type{
			READ = 1
		};
		EventLoop();
		void watch(SocketTcp * sock, ev_type ev, ev_cb cb);
		void run();
		void async_read(SocketTcp * sock, char * buff, std::size_t);

		void set_timer(timer t);
		std::uint64_t update_time();
	private:
		uint64_t get_ms_time(void);
		int compute_next_timeout();
		// run all due timers and execute their callbacks
		void run_timers();
		// poll for io
		void poll_io(int timeout);

	private:
		int m_efd;
		int m_sock_count;
		std::unordered_map<int, struct io_s> m_iomap; // fd -> peer

		uint64_t m_time; // global time since start of loop
		heap_timer_t m_timers; // min heap of timers

};
