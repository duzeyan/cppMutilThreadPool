cmake_minimum_required(VERSION 3.4)

project(MutilThread)

include_directories(../src)

aux_source_directory(../src SRC)
aux_source_directory(../src/test SRC)

add_executable(mutilTread_demo ${SRC})

