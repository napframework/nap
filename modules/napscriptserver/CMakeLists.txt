project(mod_napscriptserver)

find_package(rapidjson REQUIRED)
find_package(jsonrpc-lean REQUIRED)
find_package(zmq REQUIRED)

file(GLOB sources src/*.cpp)

add_library(${PROJECT_NAME} SHARED ${sources})
target_include_directories(${PROJECT_NAME} PUBLIC ${RAPIDJSON_INCLUDE_DIRS} ${JSONRPCLEAN_INCLUDE_DIRS} include-zmq ${ZMQ_INCLUDE_DIR})

target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)

if (MSVC)
    target_link_libraries(${PROJECT_NAME} napcore ${ZMQ_LIBRARIES} mod_napjsonserializer)
else()
    target_link_libraries(${PROJECT_NAME} napcore ${ZMQ_LIBRARIES} mod_napjsonserializer pthread)
endif()