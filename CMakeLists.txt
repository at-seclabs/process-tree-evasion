cmake_minimum_required(VERSION 3.20)
project(process_tree_evasion LANGUAGES CXX)

if(NOT WIN32)
    message(FATAL_ERROR "This lab requires Windows and the Windows SDK.")
endif()

add_executable(process_tree_evasion main.cpp proc.cpp)
target_compile_features(process_tree_evasion PRIVATE cxx_std_17)
target_compile_definitions(process_tree_evasion PRIVATE UNICODE _UNICODE)
target_link_libraries(process_tree_evasion PRIVATE advapi32)

if(MSVC)
    target_compile_options(process_tree_evasion PRIVATE /W4 /utf-8)
endif()
