#include <gtest/gtest.h>
#include <climits>
#include "bencode.h"

std::string get_random_str(int len){
	srand(time(NULL));
	char base_c = 'a';
	std::string ans = "";
	for(int i = 0; i < len; i++){
		ans += std::string(1, base_c + (rand() % 26));
	}
	return ans;
};

TEST(Bencode_Decoder, int_test){
	std::string input = "i10e";
	Bencode::Decoder decoder;
	decoder.set_bencode(input);

	decoder.set_bencode("ie30");
	EXPECT_THROW(decoder.decode(), std::invalid_argument);

	decoder.set_bencode("30ie");
	EXPECT_THROW(decoder.decode(), std::invalid_argument);

	decoder.set_bencode("i10asde");
	EXPECT_THROW(decoder.decode(), std::invalid_argument);

	decoder.set_bencode("ie");
	EXPECT_THROW(decoder.decode(), std::invalid_argument);

	decoder.set_bencode("i2sdsa2e");
	EXPECT_THROW(decoder.decode(), std::invalid_argument);

	decoder.set_bencode("10000000000000000");
	EXPECT_THROW(decoder.decode(), std::invalid_argument);

    srand(time(NULL));

	int x;
	std::string str;
	std::shared_ptr<struct Bencode::Bnode> int_data;
	for(int i = 0; i < 1000; i++){
		x = rand() % INT_MAX;
		str = "i" + std::to_string(x) + "e";
		decoder.set_bencode(str);
		int_data = decoder.decode();
		ASSERT_EQ(x, std::get<int>(int_data->m_val));
	};

};


TEST(Bencode_Decoder, string_test){
	Bencode::Decoder decoder;

	int len;
	std::string original_string, bencode_string;
	for(int i = 0; i < 1000; i++){
		len = rand() % 1000;
		original_string = get_random_str(len);
		bencode_string = std::to_string(len) + ":" + original_string;
		decoder.set_bencode(bencode_string);
		std::shared_ptr<struct Bencode::Bnode> data = decoder.decode();
		ASSERT_EQ(std::get<std::string>(data->m_val), original_string);
	}

	/*Specified length is shorter than the actual length*/
	int smaller_len;
	for(int i = 0; i < 1000; i++){
		len = rand() % 1000;
		smaller_len = len - (rand() % 1000);
		while(smaller_len < 0){
			smaller_len = len - (rand() % 1000);
		};
		original_string = get_random_str(smaller_len);
		bencode_string = std::to_string(len) + ":" + original_string;
		decoder.set_bencode(bencode_string);
		ASSERT_THROW(decoder.decode(), std::invalid_argument);
	};

};

int main(int argc, char **argv){
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
