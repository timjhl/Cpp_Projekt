cmake_minimum_required(VERSION 3.10)
project(projektabgabe)

set(SOURCE source/main.cpp source/BatchGenerator.cpp)
set(HEADER includes/header.h includes/BatchGenerator.h)
set(NAME_EXE genbatch)

add_executable(${NAME_EXE} ${SOURCE} ${HEADER})

target_link_libraries(genbatch PRIVATE jsoncpp)