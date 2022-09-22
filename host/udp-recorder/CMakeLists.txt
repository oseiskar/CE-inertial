cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wextra -O2")

project(udp_recorder)
add_executable(${PROJECT_NAME} src/main.cpp src/conversion.cpp)

option (BUILD_TESTING "Build testing" ON)
set(BUILD_TESTING OFF)
set(USE_OPENCV_VIDEO_RECORDING OFF)

add_subdirectory("jsonl-recorder")

target_link_directories(${PROJECT_NAME} PRIVATE jsonl-recorder)
target_link_libraries(${PROJECT_NAME} PRIVATE jsonl-recorder)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR} )
