cmake_minimum_required(VERSION 3.20)
project(reg-tests)

add_executable(${PROJECT_NAME} tests.cpp)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

# Set warning level
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -pedantic-errors -Wconversion -Wsign-conversion
        -Wno-unknown-pragmas # Allows us to ignore the "pessimizing-move" warning
    )
endif()

if(WARNINGS_AS_ERRORS_FOR_REG)
    if(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE /WX)
    else()
        target_compile_options(${PROJECT_NAME} PRIVATE -Werror)
    endif()
endif()

add_subdirectory(.. ${CMAKE_CURRENT_SOURCE_DIR}/build/reg)
target_link_libraries(${PROJECT_NAME} PRIVATE reg::reg)

include(FetchContent)

# ---Add doctest---
FetchContent_Declare(
    doctest
    GIT_REPOSITORY https://github.com/doctest/doctest
    GIT_TAG b7c21ec5ceeadb4951b00396fc1e4642dd347e5f
)
FetchContent_MakeAvailable(doctest)
target_link_libraries(${PROJECT_NAME} PRIVATE doctest::doctest)

# ---Add ser20---
FetchContent_Declare(
    ser20
    GIT_REPOSITORY https://github.com/CoolLibs/ser20
    GIT_TAG 0561388223ee618545b9febf7604c61f39f8aa28
)
FetchContent_MakeAvailable(ser20)
target_link_libraries(${PROJECT_NAME} PRIVATE ser20::ser20)

# ---Ignore .vscode/settings.json in Git---
find_package(Git QUIET)

if(GIT_FOUND)
    get_filename_component(PARENT_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)

    if(EXISTS "${PARENT_DIR}/.git")
        execute_process(COMMAND ${GIT_EXECUTABLE} update-index --assume-unchanged .vscode/settings.json
            WORKING_DIRECTORY ${PARENT_DIR}
            RESULT_VARIABLE ERRORS)

        if(NOT ERRORS EQUAL "0")
            message("Git assume-unchanged failed: ${ERRORS}")
        endif()
    else()
        message("No Git repository found.")
    endif()
else()
    message("Git executable not found.")
endif()