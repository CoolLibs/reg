cmake_minimum_required(VERSION 3.8)

add_library(odb INTERFACE)
add_library(odb::odb ALIAS odb)
target_compile_features(odb INTERFACE cxx_std_20)

target_include_directories(odb INTERFACE include)

set(UUID_BUILD_TESTS      OFF CACHE BOOL "")
set(UUID_SYSTEM_GENERATOR ON  CACHE BOOL "")
set(UUID_TIME_GENERATOR   OFF CACHE BOOL "")
set(UUID_USING_CXX20_SPAN ON  CACHE BOOL "")
add_subdirectory(lib/stduuid)
target_link_libraries(odb INTERFACE stduuid)