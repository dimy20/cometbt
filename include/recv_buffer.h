#pragma once
#include "buffer.h"
#include <iostream>
class RecvBuffer{
	public:
		int message_size() const { return m_message_size; };

		// how many messsage bytes are yet unprocessed
		int message_bytes_left() const { return m_message_size - m_recv_pos; };

		// maximum bytes we can receive right now
		int max_receive();

		// have we advance through the entire message?
		bool is_message_finished() const { return m_recv_pos >= m_message_size; };

		//current postion
		int pos() const { return m_recv_pos;};

		// advance read position in protocol message
		int advance_pos(int bytes);

		bool pos_at_end() { 
			return m_recv_pos == m_recv_end; 
		};

		//underlying buffer max capacity
		int capacity() { return m_buff.size(); };

		//reserve size bytes for a messsage, and return this chunk
		std::pair<char *, std::size_t > reserve(int size);
		void grow();

		// account for received bytes
		void received(int bytes_received){
			assert(m_message_size > 0);
			m_recv_end += bytes_received;
			assert(m_recv_pos <= m_buff.size());
		};



		// cut 'size bytes' from buffer, these bytes must have been processed already 
		// at the time of this call.
		void cut(int size, int next_message_size, int offset = 0);

		// chunk between start of protocol message and current read position so far
		std::pair<char *, std::size_t> get();

		//get the buffer ready to work with a new message, this cuts from the buffer
		//the current message if we have received it all at the time we call this 
		//function or else if we have not, it will simply reset the pointer values
		//to 0 and set the new message size.
		void reset(int next_message_size);

		// clean processed protocol messages
		void clean();

	private:
		// the underlying buffer where the data will be written to from the socket.
		Buffer m_buff; 
		// start of protocol message.
		int m_recv_start = 0;
		// end of protocol message
		int m_recv_end = 0;
		// current read position in protocol message
		int m_recv_pos = 0;
		// protocol message size
		int m_message_size = 0; // message size we are currently receiving

};
