project(mod_napscriptserver)

find_package(PythonLibs REQUIRED)

#find_package(Boost REQUIRED)

file(GLOB sources src/*.cpp)
include_directories(src jsonrpc_include rapidjson_include ${PYTHON_INCLUDE_DIRS} ../../../asio/include)

add_library(${PROJECT_NAME} SHARED ${sources})
target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)


target_link_libraries(${PROJECT_NAME} napcore zmq pthread ${PYTHON_LIBRARIES})