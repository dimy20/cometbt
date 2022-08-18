#include "event_loop.h"

void event_loop::set_socket(socket_tcp * sock, ev_cb read_cb){
	int fd = sock->get_fd();
	if(m_iomap.find(fd) != m_iomap.end()){
		auto& io = m_iomap[fd];
		assert(io.read_cb != nullptr);
		assert(io.sock == sock);
		return;
	}else{
		m_iomap[fd] = {sock, read_cb, nullptr, 0};
		m_sock_count++;
	}
}
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

void event_loop::event_ctl(socket_tcp * sock, std::uint32_t events){
	struct epoll_event ev;
	int err, fd;
	fd = sock->get_fd();
	if(m_iomap.find(fd) == m_iomap.end()) return;

	memset(&ev, 0, sizeof(struct epoll_event));

	ev.data.fd = fd;
	ev.events = events | EPOLLET; // use edge triggered mode

	err = epoll_ctl(m_efd, EPOLL_CTL_ADD, fd, &ev);

	if(err == -1){
		if(errno == EEXIST){
			err = epoll_ctl(m_efd, EPOLL_CTL_MOD, fd, &ev);
			if(err == -1) perror("epoll_ctl");
		}else{
			perror("epoll_ctl");
		}

	};
};

void event_loop::poll_io(int timeout){
    struct epoll_event ev[m_sock_count];
	memset(ev, 0, sizeof(struct epoll_event) * m_sock_count); 
	int n;

	if(m_sock_count == 0) return;

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
	while(!m_timers.empty() || m_sock_count > 0){
		int timeout = compute_next_timeout();
		// poll for io
		poll_io(timeout);
		// update global time
		update_time();
		// run due timers
		run_timers();
	}
};

void event_loop::async_read(socket_tcp * sock, char * buff, std::size_t size){
	const int fd = sock->get_fd();
	if(m_iomap.find(fd) != m_iomap.end()){
		auto& io = m_iomap[sock->get_fd()];
		io.buff = buff;
		io.size = size;
	}else std::cerr << "loop is not aware of this socket" << std::endl;

};

void event_loop::async_write(socket_tcp * sock, char * buff, int size){
	if(buff == nullptr || size == 0) return;


	const int fd = sock->get_fd();
	int err;
	// try to write
	int n = sock->send(buff, size, err);

	if(err == -1){
		if(errno == EAGAIN || errno == EWOULDBLOCK){
			std::uint32_t events = 0;
			if(m_iomap.find(fd) != m_iomap.end()){
				events = get_sock_events(sock);
			}
			events |= EPOLLOUT;
			event_ctl(sock, events);
			assert(m_iomap.find(fd) != m_iomap.end());

			auto& io = m_iomap[fd];
			io.write_queue.push({buff + n, size - n});
		}else perror("epoll_ctl");
	}
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


std::uint32_t event_loop::get_sock_events(socket_tcp * sock){
	assert(sock != nullptr);
	int fd = sock->get_fd();
	if(m_iomap.find(fd) == m_iomap.end()) return 0;
	auto& io = m_iomap[fd];
	return io.events;
};
