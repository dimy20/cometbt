#include <gtest/gtest.h>
#include <string.h>
#include "info_hash.h"

std::vector<char> generate_random_data(int n){
	std::vector<char> ans;
	for(int i = 0; i < n; i++){
		ans.push_back(rand() % 256);
	};
	return ans;
};

std::vector<aux::info_hash> get_hashes(int n){
	std::vector<aux::info_hash> ans;
	int data_size= 512;
	for(int i = 0; i < n; i++){
		auto data = generate_random_data(data_size);
		int m = data.size();
		ans.push_back(aux::info_hash(data.data(), m));
	}
	return ans;
};

TEST(hash, hash_copy){
	auto hash = get_hashes(1)[0];
	aux::info_hash h;
	h = hash;
	ASSERT_TRUE(h.get() != hash.get());
	ASSERT_TRUE(h == hash);
	ASSERT_TRUE(h.hex_str() != "");
	ASSERT_EQ(h.hex_str().size(), 40 + 20);
};

TEST(hash, hash_move){
	auto hash = get_hashes(1)[0];
	aux::info_hash h;
	const unsigned char * tmp = hash.get();
	h = std::move(hash);

	ASSERT_EQ(hash.get(), nullptr);
	ASSERT_TRUE(h.get() != nullptr);
	ASSERT_TRUE(memcmp(tmp, h.get(), 20) == 0);
	ASSERT_TRUE(h.hex_str() != "");
	ASSERT_EQ(h.hex_str().size(), 40 + 20);
};

TEST(hash, empty){
	auto hash = get_hashes(1)[0];
	aux::info_hash h;
	ASSERT_TRUE(h.get() == nullptr);
	ASSERT_TRUE(h.hex_str() == "");
	ASSERT_FALSE(h == hash);
};

int main(int argc, char ** argv){
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
};
