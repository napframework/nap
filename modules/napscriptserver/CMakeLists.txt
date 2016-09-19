project(mod_napscriptserver)

file(GLOB sources src/*.cpp)
include_directories(src chai_include jsonrpc_include)

add_library(${PROJECT_NAME} SHARED ${sources})
target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)

target_link_libraries(${PROJECT_NAME} napcore thirdparty zmq pthread)