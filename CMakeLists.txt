cmake_minimum_required(VERSION 3.14)
project(TorrentClient VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

file(GLOB_RECURSE SRC_FILES src/*.cpp)
find_package(OpenSSL REQUIRED)

add_library(decoder STATIC src/bencode.cpp)
target_include_directories(decoder PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(event_loop STATIC  src/socketSSL.cpp
							   src/tcp.cpp
							   src/event_loop.cpp
							   src/timer.cpp
							   src/http_parser.cpp)

target_include_directories(event_loop PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(torrent STATIC src/torrent.cpp
						   src/buffer.cpp
						   src/peer_connection_core.cpp
						   src/peer_connection.cpp
						   src/serial.cpp
						   src/session.cpp
						   src/recv_buffer.cpp
						   src/info_hash.cpp
						   src/bitfield.cpp
						   src/piece.cpp
						   src/piece_manager.cpp
						   src/block.cpp)

target_link_libraries(torrent decoder)
target_include_directories(torrent PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(main main.cpp)
target_link_libraries(main decoder
						   torrent
						   event_loop
						   crypto
						   pthread
						   ${OPENSSL_LIBRARIES})

target_include_directories(main PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
									   ${OPENSSL_INCLUDE_DIR})

include(CTest)
include(GoogleTest)

add_subdirectory(tests)

enable_testing()
