project(mod_napscriptserver)

find_package(rapidjson REQUIRED)
find_package(jsonrpc-lean REQUIRED)
find_package(asio REQUIRED)

file(GLOB sources src/*.cpp)
include_directories(src ${PYTHON_INCLUDE_DIRS})

add_library(${PROJECT_NAME} SHARED ${sources})
include_directories(${RAPIDJSON_INCLUDE_DIRS} ${JSONRPCLEAN_INCLUDE_DIRS})

target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)


target_link_libraries(${PROJECT_NAME} napcore zmq pthread)