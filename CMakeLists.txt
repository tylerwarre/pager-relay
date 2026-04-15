cmake_minimum_required(VERSION 3.28)

project(pager-relay)
file(GLOB SRCS ./**/*.c)


add_executable(pager-relay ${SRCS})
target_include_directories(pager-relay PRIVATE ./src/include/ PUBLIC ./include/)

# libcurl
find_package(CURL REQUIRED)
target_link_libraries(pager-relay PRIVATE CURL::libcurl)
target_include_directories(pager-relay PRIVATE ${CURL_INCLUDE_DIRS})

# json-c
find_package(json-c REQUIRED)
target_link_libraries(pager-relay PRIVATE json-c::json-c)
target_include_directories(pager-relay PRIVATE ${CURL_INCLUDE_DIRS})