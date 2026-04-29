cmake_minimum_required(VERSION 3.28)

#set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(pager-relay)
file(GLOB SRCS src/*.c)
file(GLOB SRCS_BRIGHT src/brightwheel/*.c)
message(STATUS "SRCS: ${SRCS}")

add_executable(pager-relay ${SRCS} ${SRCS_BRIGHT})
target_compile_options(pager-relay PRIVATE -g)
target_include_directories(pager-relay PRIVATE ./src/include/ PUBLIC ./include/ PRIVATE ./src/brightwheel/include/)

# libcurl
find_package(CURL REQUIRED)
target_link_libraries(pager-relay PRIVATE CURL::libcurl)
target_include_directories(pager-relay PRIVATE ${CURL_INCLUDE_DIRS})

# json-c
find_package(json-c CONFIG REQUIRED)
target_link_libraries(pager-relay PRIVATE json-c::json-c)

# pcre2
find_package(pcre2 CONFIG REQUIRED)
target_compile_definitions(pager-relay PRIVATE PCRE2_CODE_UNIT_WIDTH=8)
target_link_libraries(pager-relay PRIVATE PCRE2::POSIX PCRE2::8BIT)

# Copy settings file to build output
add_custom_command(
    TARGET pager-relay
    POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy
        "${CMAKE_SOURCE_DIR}/config/settings.json"
        "${CMAKE_BINARY_DIR}/settings.json"
)


# Copy compile_commands.json to project root for neovim
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_custom_command(
        TARGET pager-relay
        POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy
            "${CMAKE_BINARY_DIR}/compile_commands.json"
            "${CMAKE_SOURCE_DIR}/compile_commands.json"
    )
endif()
