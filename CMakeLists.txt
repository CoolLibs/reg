cmake_minimum_required(VERSION 3.8)

add_library(odb src/Uuid.cpp)
add_library(odb::odb ALIAS odb)
target_compile_features(odb PUBLIC cxx_std_20)

target_include_directories(odb INTERFACE include)