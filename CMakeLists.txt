cmake_minimum_required(VERSION 3.14)
project(bow LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(PkgConfig)
pkg_check_modules(GTKMM gtkmm-3.0)

link_directories(${GTKMM_LIBRARY_DIRS})
include_directories(
  ${GTKMM_INCLUDE_DIRS}
  libs/tree-sitter/lib/src
  libs/tree-sitter/lib/include)

add_library(tree_sitter
  libs/tree-sitter/lib/src/lib.c
  libs/tree-sitter-javascript/src/scanner.c
  libs/tree-sitter-javascript/src/parser.c)

target_include_directories(tree_sitter PUBLIC
  libs/tree-sitter/lib/src
  libs/tree-sitter/lib/include)

add_executable(bow main.cpp)
target_link_libraries(bow ${GTKMM_LIBRARIES} tree_sitter)

# cmake -S . -B _build -D CMAKE_EXPORT_COMPILE_COMMANDS=ON
