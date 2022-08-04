#include "event_loop.h"

uint64_t event_loop::get_ms_time(void){
	int ret;
	int err;
	struct timeval tv;

	memset(&tv, 0, sizeof(tv));
	err = gettimeofday(&tv, nullptr );
	if(err < 0) return err;
	
	ret = tv.tv_sec * 1000 + tv.tv_usec / 1000;
	return ret;
};

bool compare(timer a, timer b){
	return a.m_clamped_timeout > b.m_clamped_timeout;
};

event_loop::event_loop(){
	m_efd = epoll_create1(0);
	if(m_efd == -1) perror("epoll_create");
	m_sock_count = 0;
	m_timers = heap_timer_t(compare);
};

void event_loop::watch(SocketTcp * sock, ev_type ev, ev_cb cb){
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

void event_loop::poll_io(int timeout){
    struct epoll_event ev[m_sock_count];
	memset(ev, 0, sizeof(struct epoll_event) * m_sock_count); 
	int n;

	do{
		n = epoll_wait(m_efd, ev, m_sock_count, timeout);
	}while(n < 0 && errno == EINTR);

	if(n == -1) {
		perror("epoll_wait");
		exit(EXIT_FAILURE);
	}

	for(int i = 0; i < n; i++){
		if(ev[i].events & EPOLLIN){
			auto& io = m_iomap[ev[i].data.fd];
			int received_bytes = io.sock->recv(io.buff, io.size);
			io.cb(io.sock, io.buff, received_bytes);
		}
	}
};

void event_loop::run(){
	while(1){
		int timeout = compute_next_timeout();
		// poll for io
		poll_io(timeout);
		// update global time
		update_time();
		// run due timers
		run_timers();
	}
};

void event_loop::async_read(SocketTcp * sock, char * buff, std::size_t size){
	const int fd = sock->get_fd();
	if(m_iomap.find(fd) != m_iomap.end()){
		auto& io = m_iomap[sock->get_fd()];
		io.buff = buff;
		io.size = size;
	}else std::cerr << "loop is not aware of this socket" << std::endl;

};

int event_loop::compute_next_timeout(){
	uint64_t diff = 0;

	if(m_timers.empty()) return -1; // block indefinetly

	auto timer = m_timers.top();

	auto clamped_timeout = timer.get_expiry();

	// timer expired, return inmediatly
	if(clamped_timeout <= m_time) return 0;

	// this is how log we should block
	diff = clamped_timeout - m_time;
	
	return diff > INT_MAX ? INT_MAX : diff;

};

std::uint64_t event_loop::update_time(){
	m_time = get_ms_time();
	return m_time;
};

void event_loop::set_timer(timer t){
	// add timers to some stagin area?
	// and only when run fires push all of them to the timer hep?
	int time = update_time();
	t.start(time);
	m_timers.push(t);
};

void event_loop::run_timers(){
	while(!m_timers.empty()){
		auto timer = m_timers.top();

		//still running
		if(timer.get_expiry() > m_time) break;

		//timer is due
		timer.run_callback();

		//delete
		m_timers.pop();

		//maybe repeat
		int loop_time = update_time();
		if(timer.repeat()){
			timer.start(loop_time);
			m_timers.push(timer);
		}
	};
};

