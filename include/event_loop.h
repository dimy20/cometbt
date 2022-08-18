#pragma once
#include <sys/epoll.h>
#include <unordered_map>
#include <sys/time.h>
#include <queue>
#include <climits>
#include "tcp.h"
#include "timer.h"

typedef void(* ev_cb)(socket_tcp * sock, char * buff, std::size_t );

struct io_s{
	socket_tcp * sock;
	ev_cb read_cb;
	char * buff;
	std::size_t size;
	std::queue<std::pair<char *, int>> write_queue;
	std::uint32_t events;
};


bool compare(timer a, timer b);

class event_loop{
	typedef std::priority_queue<timer, std::vector<timer>,decltype(&compare)> heap_timer_t;
	public:
		event_loop();
		void event_ctl(socket_tcp * sock, std::uint32_t events);
		void run();
		void async_read(socket_tcp * sock, char * buff, std::size_t);
		void async_write(socket_tcp * sock, char * buff, int size);

		void set_timer(timer t);
		void set_socket(socket_tcp * sock, ev_cb);

		std::uint64_t update_time();

		std::uint32_t get_sock_events(socket_tcp * sock);
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
