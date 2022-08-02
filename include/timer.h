#pragma once
#include <cstdint>
#include <cassert>

typedef void(*timer_cb_t)(void);

class EventLoop;

class timer{
	friend class EventLoop;
	public:
		timer(int timeout_ms, timer_cb_t cb);
		timer(int timeout_ms, timer_cb_t cb, int repeat);
		timer(const timer& other) = default;
		//timer& operator=(const timer& other);
		//timer(timer && other);
		int get_expiry() { return m_clamped_timeout; } ;


	private:
		int start(int loop_time);
		/*
		void top(loop_t * loop, min_timer_t * timer);

		int is_running(min_timer_t * timer);
		*/
	protected:
		friend bool compare(timer a, timer b);
		bool repeat();
		void run_callback(){ 
			assert(m_cb != nullptr);
			m_cb();
		};

	private:
		timer_cb_t m_cb = nullptr;
		int m_clamped_timeout = 0;
		int m_timeout_ms = 0;
		int m_repeat = 0;
		uint8_t m_flags = 0; // what for?

		//cometev::loop * m_loop = nullptr;
};

