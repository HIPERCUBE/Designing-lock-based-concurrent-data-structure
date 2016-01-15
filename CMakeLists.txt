cmake_minimum_required(VERSION 3.2)
project(CppConcurrencyChapter6)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp)
add_executable(CppConcurrencyChapter6 ${SOURCE_FILES} Listing/Listing6_1.cpp Listing/Listing6_2.cpp Listing/Listing6_3.cpp Listing/Listing6_4.cpp Listing/Listing6_5.cpp Listing/Listing6_6.cpp)