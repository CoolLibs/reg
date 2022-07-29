cmake_minimum_required(VERSION 3.8)
project(reg-tests)

add_executable(${PROJECT_NAME} test.cpp)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /WX /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Werror -Wall -Wextra -Wpedantic -pedantic-errors -Wconversion -Wsign-conversion)
endif()

add_subdirectory(.. ${CMAKE_CURRENT_SOURCE_DIR}/build/reg)
target_link_libraries(${PROJECT_NAME} PRIVATE reg::reg)

add_subdirectory(doctest)
target_link_libraries(${PROJECT_NAME} PRIVATE doctest::doctest)