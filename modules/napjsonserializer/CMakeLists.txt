project(mod_napjsonserializer)

file(GLOB sources src/*.cpp src/*.h)

find_package(rapidjson REQUIRED)

add_definitions(-DRAPIDJSON_HAS_STDSTRING=1)

add_library(${PROJECT_NAME} STATIC ${sources})
include_directories(${RAPIDJSON_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PUBLIC src)

target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)
target_link_libraries(${PROJECT_NAME} napcore)

