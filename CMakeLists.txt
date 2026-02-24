cmake_minimum_required(VERSION 3.16)
project(uJSON CXX)

# Options
option(UJSON_BUILD_EXAMPLES "Build uJSON examples" ON)
option(UJSON_BUILD_SHARED "Build uJSON as shared library" OFF)

# Sources
set(UJSON_SOURCES src/ujson.cpp)

# Library
if(UJSON_BUILD_SHARED)
    add_library(ujson SHARED ${UJSON_SOURCES})
    target_compile_definitions(ujson PRIVATE UJSON_EXPORT)
else()
    add_library(ujson STATIC ${UJSON_SOURCES})
    target_compile_definitions(ujson PUBLIC UJSON_STATIC)
endif()

# Alias for submodule usage
add_library(uJSON::ujson ALIAS ujson)

# Include Directories
target_include_directories(ujson PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Example
if(UJSON_BUILD_EXAMPLES)
    add_executable(ujson_example examples/main.cpp)
    target_link_libraries(ujson_example PRIVATE ujson)
endif()
