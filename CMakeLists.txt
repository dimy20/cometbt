cmake_minimum_required(VERSION 3.14)
project(TorrentClient VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

file(GLOB_RECURSE SRC_FILES src/*.cpp)
find_package(OpenSSL REQUIRED)

add_library(decoder STATIC src/bencode.cpp)
target_include_directories(decoder PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(torrent STATIC src/torrent.cpp src/socketSSL.cpp src/tcp.cpp)
target_link_libraries(torrent decoder)
target_include_directories(torrent PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(main main.cpp)
target_link_libraries(main decoder torrent crypto ${OPENSSL_LIBRARIES})
target_include_directories(main PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
									   ${OPENSSL_INCLUDE_DIR})

include(CTest)
include(GoogleTest)

add_subdirectory(tests)

enable_testing()
