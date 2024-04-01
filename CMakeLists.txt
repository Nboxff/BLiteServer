cmake_minimum_required(VERSION 3.10)

# set the project name
project(BLiteServer VERSION 1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(GLOB SOURCES src/*.cpp)

add_executable(server ${SOURCES} test/server.cpp)
add_executable(client ${SOURCES} test/client.cpp)

target_include_directories(server PUBLIC src/include)



