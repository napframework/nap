project(mod_napscriptserver)

file(GLOB sources src/*.cpp src-mongoose/*.cpp src-mongoose/mongoose/*.cpp)
include_directories(src chai_include jsonrpc_include rapidjson-include)

add_library(${PROJECT_NAME} SHARED ${sources})
target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)

target_link_libraries(${PROJECT_NAME} napcore zmq pthread)