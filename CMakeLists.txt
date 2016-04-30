cmake_minimum_required(VERSION 2.8)
project(esexpr)

include_directories("${PROJECT_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}/PEGTL")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "-Wall")

enable_testing()

subdirs(examples tests)
