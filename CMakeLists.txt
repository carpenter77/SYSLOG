cmake_minimum_required(VERSION 2.8)
project(syslog)

set(CMAKE_VERBOSE_MAKEFILE ON)
add_definitions("-O0 -g -ggdb")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
# 指定编译器
# CMAKE_C_FLAGS_DEBUG          ----  C 编译器
# CMAKE_CXX_FLAGS_DEBUG        ----  C++ 编译器
# -std=c++11  使用 C++11
# -g：只是编译器，在编译的时候，产生调试信息。
# -Wall：生成所有警告信息。一下是具体的选项，可以单独使用
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11   -g  -Wall  ")

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/thirdlib/yaml-cpp/include/)
link_directories(${PROJECT_SOURCE_DIR}/thirdlib/yaml-cpp/lib)

#查找.下的所有cpp文件，然后将结果存进指定变量名(这里是DIR_SRCS)
AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR} DIR_SRCS)
add_library(syslog SHARED ${DIR_SRCS}) 
set_target_properties(syslog PROPERTIES LINKER_LANGUAGE CXX)
# add_subdirectory(tests)

add_executable(test_log tests/test_log.cc)
add_dependencies(test_log syslog)
target_link_libraries(test_log syslog)

add_executable(test_config tests/test_config.cc)
add_dependencies(test_config syslog)
target_link_libraries(test_config syslog yaml-cpp)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)