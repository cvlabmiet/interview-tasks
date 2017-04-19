cmake_minimum_required(VERSION 3.2)
project(interview_tasks CXX)

add_compile_options(-std=c++1z)

find_program(SPHINX_BUILD NAMES sphinx-build sphinx-build.py)
if(NOT SPHINX_BUILD)
    message(FATAL_ERROR "Could not find sphinx-build program")
endif()

configure_file(conf.py.in ${CMAKE_CURRENT_BINARY_DIR}/conf.py @ONLY)

add_custom_target(${PROJECT_NAME} ALL
    ${SPHINX_BUILD} -b html -c ${CMAKE_CURRENT_BINARY_DIR} "${CMAKE_CURRENT_SOURCE_DIR}" ${CMAKE_CURRENT_BINARY_DIR}
)

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
