#include <gtest/gtest.h>
#include "recv_buffer.h"
TEST(recv_buffer, default_constructor){
	RecvBuffer b;
	b.cut(0, 10);
	ASSERT_EQ(b.message_size(), 10);
	ASSERT_EQ(b.message_bytes_left(), 10);
	ASSERT_FALSE(b.is_message_finished());
	ASSERT_EQ(b.pos(), 0);
	ASSERT_EQ(b.capacity(), 0);
};

TEST(recv_buffer, pos_at_end_false){
	RecvBuffer b;
	b.cut(0, 1000);
	b.reserve(1000);

	b.received(1000);
	b.advance_pos(999);
	ASSERT_FALSE(b.pos_at_end());
}

TEST(recv_buffer, pos_at_end_true){
	RecvBuffer b;
	b.cut(0, 1000); // packet size
	b.reserve(1000); // reserve 
	b.reserve(1000); // no effect
	b.received(1000); // let buffer now we have received
	b.advance_pos(1000);
	ASSERT_TRUE(b.pos_at_end());
}

TEST(recv_buffer, packet_finished){
	RecvBuffer b;
	b.cut(0, 10); // packet size
	b.reserve(1000); // reserve 
	b.reserve(1000); // no effect

	b.received(1000);
	for(int i = 0; i < 10; i++){
		ASSERT_FALSE(b.is_message_finished());
		b.advance_pos(1);
	}
	ASSERT_TRUE(b.is_message_finished());
}

TEST(recv_buffer, grow_packet){
	RecvBuffer b;
	b.reset(500);
	b.grow(10000);

	ASSERT_TRUE(b.capacity() >= 500);
	ASSERT_TRUE(b.capacity() <  500 + 250);
}

TEST(recv_buffer, grow){
	RecvBuffer b;
	b.reserve(100);
	b.grow(1000000);

	ASSERT_TRUE(b.capacity() >= 150);
	ASSERT_TRUE(b.capacity() < 150 + 500);
};

TEST(recv_buffer, grow_2){
	RecvBuffer b;
	b.reserve(2000);
	b.grow(2100);

	ASSERT_TRUE(b.capacity() >= 2100);
	ASSERT_TRUE(b.capacity() < 2100 + 200);
};

TEST(recv_buffer, reserve_grow_minimum){
	RecvBuffer b;
	b.reset(1000);
	b.reserve(20);

	ASSERT_TRUE(b.capacity() >= 1000);
	ASSERT_TRUE(b.capacity() < 1000 + 500);
}

TEST(recv_buffer, reserve_grow){
	RecvBuffer b;
	b.reserve(20);

	ASSERT_TRUE(b.capacity() >= 20);
	ASSERT_TRUE(b.capacity() < 20 + 100);
}

TEST(recv_buffer, reserve){
	RecvBuffer b;
	auto [range1, size1] = b.reserve(100);
	const int cap = b.capacity();

	b.reset(20); // set packet size
	b.received(20); // received 20 bytes

	ASSERT_EQ(b.capacity(), cap);

	auto [range2, size2] = b.reserve(50);

	ASSERT_EQ(b.capacity(), cap);

	ASSERT_EQ(range1 + 20, range2);
	ASSERT_TRUE(size1 >= 20);
	ASSERT_TRUE(size2 >= 50);
}

TEST(recv_buffer, reserve_needs_grow){
	RecvBuffer b;
	b.reserve(100);

	const int cap = b.capacity();

	b.reset(40);
	b.received(40);

	ASSERT_EQ(b.capacity(), cap);

	b.reserve(100);

	ASSERT_TRUE(b.capacity() > cap);
}

TEST(recv_buffer, max_receive){
	RecvBuffer b;
	b.reset(2000); // set message size
	b.reserve(2000); // reserve for message size
	b.received(2000); // acount for received
	b.clean(); // 

	b.reset(20);

	int const max_receive = b.max_receive();
	ASSERT_TRUE(max_receive == 2000);
	b.received(20);
	ASSERT_TRUE(b.max_receive() >= max_receive - 20);

}

int main(int argc, char ** argv){
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
