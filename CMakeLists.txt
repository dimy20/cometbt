cmake_minimum_required(VERSION 3.14)
project(TryingOutRayTracing VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

find_package(SDL2 REQUIRED)
find_package(GLEW 2.0 REQUIRED)


file(GLOB_RECURSE SRC_FILES src/*.cpp)

add_executable(main main.cpp ${SRC_FILES})

target_link_libraries(main ${SDL2_LIBRARIES})
target_link_libraries(main GLEW::glew)
target_link_libraries(main -lGL)

target_include_directories(main PUBLIC ${SDL2_INCLUDE_DIRS} 
									   ${CMAKE_CURRENT_SOURCE_DIR}/include
									   ${GLEW_INCLUDE_DIRS})

