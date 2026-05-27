file(GLOB TEST_SRCS ${PROJECT_SOURCE_DIR}/tests/src/*.c)

add_executable(pager-relay-tests ${TEST_SRCS} ${PROJECT_SOURCE_DIR}/src/util.c )
target_include_directories(pager-relay-tests
    PRIVATE ${PROJECT_SOURCE_DIR}/src/include/
    PUBLIC ${PROJECT_SOURCE_DIR}/include/
    PRIVATE ${PROJECT_SOURCE_DIR}/tests/src/
)

# libcurl
find_package(CURL REQUIRED)
target_link_libraries(pager-relay-tests PRIVATE CURL::libcurl)
target_include_directories(pager-relay-tests PRIVATE ${CURL_INCLUDE_DIRS})

# json-c
find_package(json-c CONFIG REQUIRED)
target_link_libraries(pager-relay-tests PRIVATE json-c::json-c)

# pcre2
find_package(pcre2 CONFIG REQUIRED)
target_compile_definitions(pager-relay-tests PRIVATE PCRE2_CODE_UNIT_WIDTH=8)
target_link_libraries(pager-relay-tests PRIVATE PCRE2::POSIX PCRE2::8BIT)

# Copy test message directory to target binary destination
add_custom_command(
    TARGET pager-relay-tests
    POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_directory
        "${CMAKE_SOURCE_DIR}/tests/msgs"
        "${CMAKE_CURRENT_BINARY_DIR}/msgs"
)
