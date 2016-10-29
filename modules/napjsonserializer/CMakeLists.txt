project(mod_napjsonserializer)

file(GLOB sources src/*.cpp)

add_library(${PROJECT_NAME} SHARED ${sources})
target_include_directories(${PROJECT_NAME} PUBLIC src include-rapidjson)

target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)
target_link_libraries(${PROJECT_NAME} napcore)

