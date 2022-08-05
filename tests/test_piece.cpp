#include <gtest/gtest.h>
#include "piece.h"

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

TEST(piece, constructor){
	int n = 100;
	auto hashes = get_hashes(n);
	std::vector<piece> pieces;
	for(int i =0 ; i < n; i++){
		pieces.push_back(piece(i, hashes[i]));
	}

	for(int i = 0; i < n; i++){
		ASSERT_TRUE(pieces[i].hash() == hashes[i]);
	};

};

int main(int argc, char **argv){
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
};
