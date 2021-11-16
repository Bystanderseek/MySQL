cmake_minimum_required(VERSION 3.20)
project(MySQL)

set(CMAKE_CXX_STANDARD 11)

add_executable(MySQL main.cpp MySQL.cpp MySQL.h Database.cpp Database.h Table.cpp Table.h utility_func.h utility_func.cpp File.cpp File.h)
