cmake_minimum_required(VERSION 3.14)
project(task5)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS: "${CMAKE_CXX_FLAGS} -Wall -Wextra -std=c++17 -O3 -pedantic")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -std=c++17 -pedantic -g -O0 -fsanitize=undefined,address -D_GLIBCXX_DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -Wextra -std=c++17 -pedantic -O3")

add_executable(task5 main.cpp parser.cpp parser.h expression.cpp hashing.cpp hashing.h expression.h debug_output.h axioms.cpp axioms.h)