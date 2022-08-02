#include "timer.h"

timer::timer(int timeout_ms, timer_cb_t cb){
	m_timeout_ms = timeout_ms;
	m_cb = cb;
	m_repeat = 0;
};

timer::timer(int timeout_ms, timer_cb_t cb, int repeat){
	m_timeout_ms = timeout_ms;
	m_cb = cb;
	m_repeat = repeat < 0 ? 0 : repeat;
};

int timer::start(int loop_time){

	uint64_t expiry;

	expiry = loop_time + m_timeout_ms;

	if(expiry < m_timeout_ms)
		expiry = (uint64_t) - 1;

	m_clamped_timeout = expiry;

	return 0;
};

bool timer::repeat(){
	bool ans = m_repeat > 0;
	m_repeat--;
	return ans;
};
