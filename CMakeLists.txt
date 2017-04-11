cmake_minimum_required(VERSION 3.2)
project(interview_tasks CXX)

add_compile_options(-std=c++1z)

find_program(RST2HTML NAMES rst2html.py rst2html)
if(NOT RST2HTML)
    message(FATAL_ERROR "Could not find rst2html.py program")
endif()

set(OUTPUT_FILE ${CMAKE_CURRENT_BINARY_DIR}/index.html)
set(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/answers.rst)
add_custom_command(OUTPUT ${OUTPUT_FILE} COMMAND ${RST2HTML} ${SOURCES} ${OUTPUT_FILE} DEPENDS ${SOURCES})
add_custom_target(${PROJECT_NAME} ALL DEPENDS ${OUTPUT_FILE})

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

add_executable(custom_main custom_main.cpp)
set_target_properties(custom_main PROPERTIES COMPILE_OPTIONS -Wno-main)
target_link_libraries(custom_main dl)

add_executable(multiloop_simple multiloop_simple.cpp)
add_executable(multiloop multiloop.cpp)

include(CTest)

add_test(NAME custom_main COMMAND custom_main -1 two -three four 5)
set_tests_properties(custom_main PROPERTIES PASS_REGULAR_EXPRESSION "argc = 6[\r\n\t ]([^\r\n]*[\r\n])+arg #3 = -three")

add_test(NAME multiloop_simple COMMAND multiloop_simple)
set_tests_properties(multiloop_simple PROPERTIES PASS_REGULAR_EXPRESSION "Received \\(2, 1, 13\\)")

add_test(NAME multiloop COMMAND multiloop)
set_tests_properties(multiloop PROPERTIES PASS_REGULAR_EXPRESSION "Received \\(2, 1, 13\\)")
