#include "recv_buffer.h"
#include <string.h>


// the maximum i can receive right now
int recv_buffer::max_receive(){
	return static_cast<int>(m_buff.size()) - m_recv_end;
}

// reserve size bytes
std::pair<char *, std::size_t> recv_buffer::reserve(int size){
	assert(m_recv_start == 0);
	assert(size > 0);
	assert(m_recv_pos >= 0);

	// check if we really need to reserve space for the size we're being asked to

	if(static_cast<int>(m_recv_end + size > m_buff.size())){
		int new_size;
		new_size = std::max(m_recv_end + size, m_message_size);
		buffer new_buffer(new_size, m_buff.data(), m_recv_end);
		m_buff = std::move(new_buffer);
	};

	return {m_buff.data() + m_recv_end, size};
}

void recv_buffer::grow(){
	int current_size = m_buff.size();

	// grow 50%
	int new_size;
	new_size = (current_size < m_message_size) ? m_message_size : current_size * 3/2;
		

	buffer new_buffer(new_size, m_buff.data(), m_recv_end);
	m_buff = std::move(new_buffer);
};

int recv_buffer::advance_pos(int bytes){
	int limit;
	// as much as possible
	limit = m_message_size > m_recv_pos ? m_message_size - m_recv_pos : m_message_size;

	int advance_amount = std::min(limit, bytes);

	m_recv_pos += advance_amount;
	return advance_amount;
};

void recv_buffer::cut(int size, int next_message_size, int offset){
	assert(size >= 0);
	assert(m_buff.size() >= size);
	assert(m_buff.size() >= m_recv_pos);
	assert(next_message_size > 0);
	// should always be true
	assert(m_recv_start <= m_recv_end);
	// no negative offset
	assert(offset >= 0);
	// make sure we have processed what we're gonna cut
	assert(m_recv_pos >= offset + size); 

	if(offset > 0){
		if(size > 0){
			char * dst = m_buff.data() + m_recv_start + offset;
			char * src = m_buff.data() + m_recv_start + offset + size;
			std::size_t remainder_size = m_recv_end - m_recv_start - (offset + size);
			memmove(dst, src, remainder_size);
		}

		m_recv_pos -= size;
		m_recv_end -= size;

	}else{
		assert(m_recv_start + size <= m_recv_end);
		m_recv_start += size;
		m_recv_pos -= size;
	}

	m_message_size = next_message_size;
};

std::pair<char *, std::size_t> recv_buffer::get(){
	if(m_buff.empty()){
		return {nullptr, 0};
	};
	return {m_buff.data() + m_recv_start, m_recv_pos};
};

void recv_buffer::reset(int next_message_size){
	assert(m_recv_end <= m_buff.size());
	assert(next_message_size > 0);

	//we have received all of the current message, so cut it from buffer
	//and get ready for the next message
	if(m_recv_end > m_message_size){
		cut(m_message_size, next_message_size);
		return;
	}else{
		//this means we have not received the enterity of the current message
		// just reset everything for next message.
		m_recv_pos = 0;
		m_recv_start = 0;
		m_recv_end = 0;
		m_message_size = next_message_size;
	}

};

void recv_buffer::clean(){
	if(m_recv_end > m_recv_start && m_recv_start > 0){
		int const size = m_recv_end - m_recv_start;
		memmove(m_buff.data(), m_buff.data() + m_recv_start, size);
	}
	m_recv_end -= m_recv_start;
	m_recv_start = 0;
}
