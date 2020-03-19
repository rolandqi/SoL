cmake_minimum_required(VERSION 2.6)

project(SoL)

set(CXX_FLAGS
 -g
 #-DCHECK_PTHREAD_RETURN_VALUE
 #-D_FILE_OFFSET_BITS=64
 -Wl,-rpath=/tools/lib64 
 -Wl,--dynamic-linker=/tools/lib/ld-2.17.so
 -Wall
 -std=c++11
 -O3 
 -pthread 
 -lrt
 -rdynamic
 )
if(CMAKE_BUILD_BITS EQUAL 32)
  list(APPEND CXX_FLAGS "-m32")
endif()
string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")

set(CMAKE_CXX_COMPILER "g++")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

find_package(Protobuf)

include_directories(${Boost_INCLUDE_DIRS})

include_directories(${PROJECT_SOURCE_DIR})

message(STATUS "CXX_FLAGS = " ${CMAKE_CXX_FLAGS} " " ${CMAKE_CXX_FLAGS_${BUILD_TYPE}})

add_subdirectory(base)
add_subdirectory(net)

add_subdirectory(Examples)


