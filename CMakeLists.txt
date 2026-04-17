cmake_minimum_required(VERSION 3.28)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(pager-relay)
file(GLOB SRCS **/*.c)
message(STATUS "SRCS: ${SRCS}")

add_executable(pager-relay ${SRCS})
target_compile_options(pager-relay PRIVATE -g)
target_include_directories(pager-relay PRIVATE ./src/include/ PUBLIC ./include/)

# libcurl
find_package(CURL REQUIRED)
target_link_libraries(pager-relay PRIVATE CURL::libcurl)
target_include_directories(pager-relay PRIVATE ${CURL_INCLUDE_DIRS})

# json-c
find_package(json-c CONFIG REQUIRED)
target_link_libraries(pager-relay PRIVATE json-c::json-c)

# Copy settings file to build output
add_custom_command(
    TARGET pager-relay
    POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy
        "${CMAKE_SOURCE_DIR}/config/settings.json"
        "${CMAKE_BINARY_DIR}/settings.json"
)

# Copy compile_commands.json to project root for neovim
add_custom_command(
    TARGET pager-relay
    POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy
        "${CMAKE_BINARY_DIR}/compile_commands.json"
        "${CMAKE_SOURCE_DIR}/compile_commands.json"
)