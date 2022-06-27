#include <gtest/gtest.h>
#include <climits>
#include "bencode.h"

std::string get_random_str(int len){
	//srand(time(NULL));
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

std::vector<std::vector<char>> random_streams(int count){
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


	Bencode::Decoder decoder;
	decoder.set_bencode(buff_str("i10e"));
	auto data = decoder.decode();
	ASSERT_EQ(std::get<int>(data->m_val), 10);

	decoder.set_bencode(buff_str("ie30"));

	EXPECT_THROW(decoder.decode(), std::invalid_argument);

	decoder.set_bencode(buff_str("30ie"));
	EXPECT_THROW(decoder.decode(), std::invalid_argument);


	decoder.set_bencode(buff_str("i10asde"));
	EXPECT_THROW(decoder.decode(), std::invalid_argument);


	decoder.set_bencode(buff_str("ie"));
	EXPECT_THROW(decoder.decode(), std::invalid_argument);


	decoder.set_bencode(buff_str("i2sdsa2e"));
	EXPECT_THROW(decoder.decode(), std::invalid_argument);


	decoder.set_bencode(buff_str("10000000000000000"));
	EXPECT_THROW(decoder.decode(), std::invalid_argument);

    srand(time(NULL));

	int x;
	std::string str;
	std::shared_ptr<struct Bencode::Bnode> int_data;
	for(int i = 0; i < 1000; i++){
		x = rand() % INT_MAX;
		str = "i" + std::to_string(x) + "e";
		decoder.set_bencode(buff_str(str));
		int_data = decoder.decode();
		ASSERT_EQ(x, std::get<int>(int_data->m_val));
	};

};



TEST(Bencode_Decoder, string_test){
	Bencode::Decoder decoder;

	int len;

	for(int i = 0; i < 100; i++){
		len = rand() % 100;
		auto stream = get_random_stream(len);
		std::string len_s = std::to_string(len) + ":";

		std::vector<char> buff(len_s.data(), len_s.data() + len_s.size());
		char * cstream = &stream[0];
		buff.insert(buff.end(), cstream, cstream + stream.size());
		decoder.set_bencode(buff);
		std::shared_ptr<struct Bencode::Bnode> data = decoder.decode();
		
		auto decoded = std::get<std::vector<char>>(data->m_val);
		
		ASSERT_EQ(decoded, stream);
	}
	//Specified length is shorter than the actual length
	int smaller_len;
	std::string msg = "";
	for(int i = 0; i < 1000; i++){
		len = rand() % 1000;
		smaller_len = len - (rand() % 1000);
		while(smaller_len < 0){
			smaller_len = len - (rand() % 1000);
		};

		auto stream = get_random_stream(smaller_len);
		std::string len_s = std::to_string(len) + ":";
		std::vector<char> buff(len_s.data(), len_s.data() + len_s.size());
		char * cstream = &stream[0];
		buff.insert(buff.end(), cstream, cstream + stream.size());

		decoder.set_bencode(buff);

		msg += "len : "  + std::to_string(len) + " ";
		msg += "s_len: " + std::to_string(smaller_len) + " ";

		if(smaller_len < len){
			ASSERT_THROW(decoder.decode(), std::invalid_argument) << msg;
		}else{
			auto data = decoder.decode();
			ASSERT_EQ(std::get<std::vector<char>>(data->m_val), stream) << msg;
		}

	};

};

TEST(Bencode_Decoder, list_test){
	Bencode::Decoder decoder;
	decoder.set_bencode(buff_str("le"));

	std::shared_ptr<struct Bencode::Bnode> data = decoder.decode();
		
	auto list = std::get<Bencode::list_t>(data->m_val);

	ASSERT_EQ(list.size(), 0);


	//test a list containing just integers
	std::vector<char> list_ints;
	list_ints.push_back('l');
	int integer;
	std::vector<int> ints(100, 0);
	for(int i = 0; i < 100; i++){
		ints[i] = rand() % 1000;
		std::string tmp = "i" + std::to_string(ints[i]) + "e";
		list_ints.insert(list_ints.end(), tmp.data(), tmp.data() + tmp.size());
	};

	list_ints.push_back('e');

	decoder.set_bencode(list_ints);
	data = decoder.decode();
	list = std::get<Bencode::list_t>(data->m_val);

	int n = list.size();
	for(int i = 0; i<n; i++){
		ASSERT_EQ(std::get<int>(list[i]->m_val), ints[i]);
	};



	//Test a list containing just strings
	int len;

	std::vector<char> list_strings;
	std::vector<std::vector<char>> strs(100);

	list_strings.push_back('l');
	for(int i = 0; i < 100; i++){
		len = rand() % 100;
		strs[i] = get_random_stream(len);
		std::string temp = std::to_string(len) + ":";
		list_strings.insert(list_strings.end(), temp.data(), temp.data() + temp.size());
		char * cstream = &strs[i][0];
		list_strings.insert(list_strings.end(), cstream, cstream + strs[i].size());
	}

	list_strings.push_back('e');
	decoder.set_bencode(list_strings);

	data = decoder.decode();
	list = std::get<Bencode::list_t>(data->m_val);

	for(int i = 0; i < list.size(); i++){
		ASSERT_EQ(std::get<std::vector<char>>(list[i]->m_val), strs[i]);
	}

	//Mix strings and ints
	std::vector<char> list_int_str;
	ints = random_ints(50);
	strs = random_streams(50);

	list_int_str.push_back('l');
	char * cstream;
	std::string str_tmp, int_tmp;
	for(int i = 0; i < 50; i++){
		int_tmp = "i" + std::to_string(ints[i]) + "e";
		list_int_str.insert(list_int_str.end(), int_tmp.data(), int_tmp.data() + int_tmp.size());

		str_tmp = std::to_string(strs[i].size()) + ":";
		list_int_str.insert(list_int_str.end(), str_tmp.data(), str_tmp.data() + str_tmp.size());
		cstream = &strs[i][0];
		list_int_str.insert(list_int_str.end(), cstream, cstream + strs[i].size());
	};
	list_int_str.push_back('e');

	decoder.set_bencode(list_int_str);
	data = decoder.decode();
	list = std::get<Bencode::list_t>(data->m_val);

	int j = 0;
	for(int i = 0; i < 100; i += 2){
		ASSERT_EQ(std::get<int>(list[i]->m_val), ints[j]);
		j++;
	}

	j = 0;
	for(int i = 1; i < 100; i += 2){
		ASSERT_EQ(std::get<std::vector<char>>(list[i]->m_val), strs[j]);
		j++;
	}

};

TEST(Bencode_Decoder, list_of_list_test){
	/*list of lists of ints*/
	Bencode::Decoder decoder;

	std::vector<char> lists;
	std::vector<int> ints(100);

	lists.push_back('l');
	for(int i = 0; i < 100; i++){
		lists.push_back('l');

		ints[i] = rand() % 1000;

		std::string tmp = "i" + std::to_string(ints[i]) + "e";
		lists.insert(lists.end(), tmp.data(), tmp.data() + tmp.size());

		lists.push_back('e');
	}

	lists.push_back('e');

	decoder.set_bencode(lists);
	auto data = decoder.decode();

	int i = 0;
	for(auto elem: std::get<Bencode::list_t>(data->m_val)){
		auto list = std::get<Bencode::list_t>(elem->m_val);
		ASSERT_EQ(std::get<int>(list[0]->m_val), ints[i]);
		i++;
	}
}

void insert_key_value(std::vector<char>& stream, const std::vector<char>& key, const std::string& value){		
	std::string tmp = std::to_string(key.size()) + ":";
	stream.insert(stream.end(), tmp.data(), tmp.data() + tmp.size());
	stream.insert(stream.end(), &key[0], &key[key.size()]);
	stream.insert(stream.end(), value.data(), value.data() + value.size());
};

void insert_key(std::vector<char>& stream, const std::vector<char>& key){
	std::string tmp = std::to_string(key.size()) + ":";
	stream.insert(stream.end(), tmp.data(), tmp.data() + tmp.size());
	stream.insert(stream.end(), &key[0], &key[key.size()]);
}

std::string int_bencode(int value){
	return "i" + std::to_string(value) + "e";
};

TEST(Bencode_Decoder, dict_test){
	Bencode::Decoder decoder;
	std::vector<char> input;

	input.push_back('d');

	std::vector<std::string> keys(20);
	std::vector<int> values(10);
	std::vector<std::vector<char>> byte_strings(10);
	for(int i = 0; i < 10; i++){
		keys[i] = get_random_str(10);
		values[i] = rand() % 100;
		insert_key_value(input, buff_str(keys[i]), int_bencode(values[i]));
	}

	for(int i = 10; i < 20 ; i++){
		keys[i] = get_random_str(10);
		byte_strings[i-10] = get_random_stream(10);

		insert_key(input, buff_str(keys[i]));

		std::string value_tmp = std::to_string(byte_strings[i-10].size()) + ":";
		input.insert(input.end(), value_tmp.data(), value_tmp.data() + value_tmp.size());

		char * cstring_val = &byte_strings[i-10][0];

		input.insert(input.end(), cstring_val, cstring_val + byte_strings[i-10].size());

	};

	input.push_back('e');

	decoder.set_bencode(input);

	auto data = decoder.decode();
	auto dict = std::get<Bencode::dict_t>(data->m_val);

	for(int i = 0; i < 10; i++){
		ASSERT_EQ(std::get<int>(dict[keys[i]]->m_val), values[i]);
	}

	for(int i = 10; i < 20; i++){
		auto bytes_string = std::get<std::vector<char>>(dict[keys[i]]->m_val);
		ASSERT_EQ(bytes_string, byte_strings[i-10]);
	}


};

int main(int argc, char **argv){
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
