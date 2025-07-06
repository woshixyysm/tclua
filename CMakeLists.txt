cmake_minimum_required(VERSION 3.10)
project(LuaInterpreter)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(LuaInterpreter main.cpp ${SOURCES})
