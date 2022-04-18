cmake_minimum_required(VERSION 3.8)

add_library(reg INTERFACE)
add_library(reg::reg ALIAS reg)
target_compile_features(reg INTERFACE cxx_std_20)

target_include_directories(reg INTERFACE include)
target_include_directories(reg INTERFACE lib) # To find cpp-typelist

set(UUID_BUILD_TESTS      OFF CACHE BOOL "")
set(UUID_SYSTEM_GENERATOR ON  CACHE BOOL "")
set(UUID_TIME_GENERATOR   OFF CACHE BOOL "")
set(UUID_USING_CXX20_SPAN ON  CACHE BOOL "")
add_subdirectory(lib/stduuid)
target_link_libraries(reg INTERFACE stduuid)