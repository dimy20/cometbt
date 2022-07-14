#include <gtest/gtest.h>
#include <time.h>
#include "buffer.h"

#define TEST_DATA_SIZE 256
std::vector<char> make_random_data(){
	std::vector<char> data(TEST_DATA_SIZE);
	for(int i = 0; i < data.size(); i++){
		data[i] = rand() % TEST_DATA_SIZE;
	};
	return data;
};

TEST(buffer, default_constructor){
	Buffer buff;
	ASSERT_EQ(buff.data(), nullptr);
	ASSERT_EQ(buff.size(), 0);
	ASSERT_TRUE(buff.empty());
	ASSERT_EQ(buff.begin(), buff.data());

};

TEST(buffer, size_constructor){
	int size = rand() % 1000;
	Buffer buffer(size);
	ASSERT_TRUE(buffer.data() != nullptr);
	ASSERT_FALSE(buffer.empty());
	ASSERT_EQ(buffer.end() - buffer.begin(), buffer.size());
	ASSERT_EQ(buffer.size(), size);
};

TEST(buffer, data_constructor){
	auto test_data = make_random_data();
	Buffer buffer(300, test_data.data(), test_data.size());
	ASSERT_FALSE(buffer.empty());
	ASSERT_TRUE(buffer.size() >= 300);
	ASSERT_TRUE(memcmp(buffer.data(), test_data.data(), test_data.size()) == 0);
};

TEST(buffer, move_constructor){
	auto test_data = make_random_data();
	Buffer b1(300, test_data.data(), test_data.size());
	ASSERT_FALSE(b1.empty());
	ASSERT_TRUE(b1.size() >= 300);
	ASSERT_TRUE(memcmp(b1.data(), test_data.data(), test_data.size()) == 0);

	Buffer b2(std::move(b1));

	// assert b2 owns data now
	ASSERT_TRUE(b1.empty());
	ASSERT_TRUE(memcmp(b2.data(), test_data.data(), test_data.size()) == 0);
	ASSERT_TRUE(b2.size() >= 300);
}

TEST(buffer, move_assigment_operator){
	auto test_data = make_random_data();
	Buffer b1(300, test_data.data(), test_data.size());
	ASSERT_FALSE(b1.empty());
	ASSERT_TRUE(b1.size() >= 300);
	ASSERT_TRUE(memcmp(b1.data(), test_data.data(), test_data.size()) == 0);

	Buffer b2;
	ASSERT_TRUE(b2.empty());

	// move assign
	b2 = std::move(b1);
	
	ASSERT_TRUE(b1.empty());

	// assert b2 owns data now
	ASSERT_FALSE(b2.empty());
	ASSERT_TRUE(b2.size() >= 300);
	ASSERT_TRUE(memcmp(b2.data(), test_data.data(), test_data.size()) == 0);

}


int main(int argc, char **argv){
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
};
