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

std::vector<char> get_random_stream(int len){
	std::vector<char> ans;
	for(int i = 0; i < len; i++){
		ans.push_back(rand() % 255);
	}
	return ans;
}

std::vector<int> random_ints(int count){
	std::vector<int> ans;
	for(int i = 0; i < count; i++){
		ans.push_back(rand() % 1000);
	};
	return ans;
};

std::vector<std::vector<char>> random_strings(int count){
	std::vector<std::vector<char>> ans;
	for(int i = 0; i < count; i++){
		ans.push_back(get_random_stream(rand() % 100));
	};
	return ans;
};

std::vector<char> buff_str(const std::string& s){
	std::vector<char> buff(s.data(), s.data() + s.size());
	return buff;
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
	std::string msg = "";
	for(int i = 0; i < 1000; i++){
		len = rand() % 1000;
		smaller_len = len - (rand() % 1000);
		while(smaller_len < 0){
			smaller_len = len - (rand() % 1000);
		};
		original_string = get_random_str(smaller_len);
		bencode_string = std::to_string(len) + ":" + original_string;
		decoder.set_bencode(bencode_string);

		msg += "len : "  + std::to_string(len) + " ";
		msg += "s_len: " + std::to_string(smaller_len) + " ";
		msg += "string : " + original_string;

		if(smaller_len < len){
			ASSERT_THROW(decoder.decode(), std::invalid_argument) << msg;
		}else{
			auto data = decoder.decode();
			ASSERT_EQ(std::get<std::string>(data->m_val), original_string) << msg;
		}

	};

};

TEST(Bencode_Decoder, list_test){
	Bencode::Decoder decoder;
	decoder.set_bencode("le");

	std::shared_ptr<struct Bencode::Bnode> data = decoder.decode();
		
	auto list = std::get<Bencode::list_t>(data->m_val);

	ASSERT_EQ(list.size(), 0);

	/*test a list containing just integers*/
	std::string bencode_list = "l";
	int integer;
	std::vector<int> ints(100, 0);
	for(int i = 0; i < 100; i++){
		ints[i] = rand() % 1000;
		bencode_list += "i" + std::to_string(ints[i]) + "e";
	};

	bencode_list += "e";

	decoder.set_bencode(bencode_list);
	data = decoder.decode();
	list = std::get<Bencode::list_t>(data->m_val);

	int n = list.size();
	for(int i = 0; i<n; i++){
		ASSERT_EQ(std::get<int>(list[i]->m_val), ints[i]);
	};

	/*Test a list containing just strings*/
	bencode_list = "l";
	int len;

	std::string bencode_string, original_string;
	std::vector<std::string> strs(100, "");
	for(int i = 0; i < 100; i++){
		len = rand() % 100;
		original_string = get_random_str(len);
		bencode_string = std::to_string(len) + ":" + original_string;
		strs[i] = original_string;
		bencode_list += bencode_string;
	}

	bencode_list += "e";

	decoder.set_bencode(bencode_list);
	data = decoder.decode();
	list = std::get<Bencode::list_t>(data->m_val);

	for(int i = 0; i < list.size(); i++){
		ASSERT_EQ(std::get<std::string>(list[i]->m_val), strs[i]);
	}

	/*Mix strings and ints*/
	bencode_list = "l";
	ints = random_ints(50);
	strs = random_strings(50);

	for(int i = 0; i < 50; i++){
		bencode_list += "i" + std::to_string(ints[i]) + "e";
		bencode_list += std::to_string(strs[i].size()) + ":" + strs[i];
	};
	bencode_list += "e";

	decoder.set_bencode(bencode_list);
	data = decoder.decode();
	list = std::get<Bencode::list_t>(data->m_val);

	int j = 0;
	for(int i = 0; i < 100; i+=2){
		ASSERT_EQ(std::get<int>(list[i]->m_val), ints[j]);
		j++;
	}

	j = 0;
	for(int i = 1; i < 100; i+=2){
		ASSERT_EQ(std::get<std::string>(list[i]->m_val), strs[j]);
		j++;
	}
};

int main(int argc, char **argv){
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
