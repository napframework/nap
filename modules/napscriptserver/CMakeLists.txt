project(mod_napscriptserver)

find_package(rapidjson REQUIRED)
find_package(jsonrpc-lean REQUIRED)

if (UNIX)
else ()
    find_package(zmq REQUIRED)
endif ()

file(GLOB sources src/*.cpp)

add_library(${PROJECT_NAME} SHARED ${sources})
if (UNIX)
    target_include_directories(${PROJECT_NAME} PUBLIC ${RAPIDJSON_INCLUDE_DIRS} ${JSONRPCLEAN_INCLUDE_DIRS} src include-zmq ${ZMQ_INCLUDE_DIR})
else ()
    target_include_directories(${PROJECT_NAME} PUBLIC ${RAPIDJSON_INCLUDE_DIRS} ${JSONRPCLEAN_INCLUDE_DIRS} include-zmq ${ZMQ_INCLUDE_DIR})
endif ()

target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)

if (MSVC)
    target_link_libraries(${PROJECT_NAME} napcore ${ZMQ_LIBRARIES} mod_napjsonserializer)
else ()
    if (UNIX)
        target_link_libraries(${PROJECT_NAME} napcore zmq mod_napjsonserializer pthread ${ZMQ_LIBRARIES})
    else ()
        target_link_libraries(${PROJECT_NAME} napcore ${ZMQ_LIBRARIES} mod_napjsonserializer pthread)
    endif ()
endif ()

if (UNIX)
else()
    foreach (RES ${ZMQ_LIBRARIES})
        file(COPY ${RES} DESTINATION ${BIN_DIR})
    endforeach()
endif()