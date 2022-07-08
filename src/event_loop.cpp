#include "event_loop.h"

EventLoop::EventLoop(){
	m_efd = epoll_create1(0);
	if(m_efd == -1) perror("epoll_create");
	m_sock_count = 0;
};

void EventLoop::watch(SocketTcp * sock, ev_type ev, ev_cb cb){
	struct epoll_event sock_ev;
	int ret;
	memset(&sock_ev, 0, sizeof(sock_ev));

	// in case connection failed
	if((sock->m_sock_state == SocketTcp::socket_state::CONNECTED)){
		switch(ev){
			case ev_type::READ:
				sock_ev.events = EPOLLIN | EPOLLET;
				break;
		}

		sock_ev.data.fd = sock->get_fd();
		ret = epoll_ctl(m_efd, EPOLL_CTL_ADD, sock->get_fd(), &sock_ev);

		if(ret == -1) perror("epoll_ctl");

		m_iomap[sock->get_fd()] = {sock, cb}; // add entry
		m_sock_count++;
	}
};

void EventLoop::run(){
    struct epoll_event ev[m_sock_count];
	memset(ev, 0, sizeof(struct epoll_event) * m_sock_count); 
	int n;
	while(1){
		n = epoll_wait(m_efd, ev, m_sock_count, -1);
		if(n == -1) perror("epoll_wait");

		for(int i = 0; i < n; i++){
			if(ev[i].events & EPOLLIN){
				auto& io = m_iomap[ev[i].data.fd];
				io.cb(io.sock);
			}
		}
	}
};
