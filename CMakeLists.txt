cmake_minimum_required(VERSION 3.20)

set(WARNINGS_AS_ERRORS_FOR_REG OFF CACHE BOOL "ON iff you want to treat warnings as errors")

add_library(reg)
add_library(reg::reg ALIAS reg)
target_compile_features(reg PUBLIC cxx_std_20)

# ---Add source files---
if(WARNINGS_AS_ERRORS_FOR_REG)
    target_include_directories(reg PUBLIC include)
else()
    target_include_directories(reg SYSTEM PUBLIC include)
endif()

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS src/*)
target_sources(reg PRIVATE ${SRC_FILES})

# ---Add stduuid library---
set(UUID_BUILD_TESTS OFF CACHE BOOL "")
set(UUID_TIME_GENERATOR OFF CACHE BOOL "")
set(UUID_USING_CXX20_SPAN ON CACHE BOOL "")
add_subdirectory(lib/stduuid)
target_link_libraries(reg PUBLIC stduuid)
install(FILES "lib/stduuid/LICENSE" DESTINATION "license/stduuid")

# ---Set warning level---
if(MSVC)
    target_compile_options(reg PRIVATE /W4)
else()
    target_compile_options(reg PRIVATE -Wall -Wextra -Wpedantic -pedantic-errors -Wconversion -Wsign-conversion)
endif()

# ---Maybe enable warnings as errors---
if(WARNINGS_AS_ERRORS_FOR_REG)
    if(MSVC)
        target_compile_options(reg PRIVATE /WX)
    else()
        target_compile_options(reg PRIVATE -Werror)
    endif()
endif()