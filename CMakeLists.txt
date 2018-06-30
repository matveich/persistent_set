cmake_minimum_required(VERSION 3.10)
project(persistent_set)

set(CMAKE_CXX_STANDARD 14)

add_executable(persistent_set
        #main.cpp
        persistent_set.cpp
        persistent_set.h
        )

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -pedantic")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address,undefined -D_GLIBCXX_DEBUG")
endif()

target_link_libraries(persistent_set -lpthread)