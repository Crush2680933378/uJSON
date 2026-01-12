# Sources
set(UJSON_SOURCES src/ujson.cpp)

# Shared Library (Default "uJSON" target)
add_library(uJSON SHARED ${UJSON_SOURCES})
target_include_directories(uJSON PUBLIC include)
target_compile_definitions(uJSON PRIVATE UJSON_EXPORT)

# Static Library
add_library(uJSON_static STATIC ${UJSON_SOURCES})
target_include_directories(uJSON_static PUBLIC include)
target_compile_definitions(uJSON_static PUBLIC UJSON_STATIC)

# Example
add_executable(ujson_example examples/main.cpp)
target_link_libraries(ujson_example PRIVATE uJSON)
