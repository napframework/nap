project(mod_napscriptserver)

find_package(rapidjson REQUIRED)
find_package(jsonrpc-lean REQUIRED)

if (APPLE)
    find_package(zmq REQUIRED)
elseif (UNIX)
else ()
    find_package(zmq REQUIRED)
endif ()

file(GLOB sources src/*.cpp src/*.h)

set(LIBRARIES napcore mod_napjsonserializer)
set(INCLUDES ${RAPIDJSON_INCLUDE_DIRS} ${JSONRPCLEAN_INCLUDE_DIRS} src include-zmq)

add_library(${PROJECT_NAME} SHARED ${sources})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)

if(WIN32)
    list(APPEND INCLUDES ${ZMQ_INCLUDE_DIR})
    if(MSVC)
        list(APPEND LIBRARIES ${ZMQ_LIBRARIES})
    else()
        list(APPEND LIBRARIES ${ZMQ_LIBRARIES} pthread)
    endif()
elseif(APPLE)
    list(APPEND INCLUDES ${ZMQ_INCLUDE_DIR})
    list(APPEND LIBRARIES ${ZMQ_LIBRARIES})
elseif(UNIX)
    list(APPEND LIBRARIES zmq pthread)
endif()

add_definitions(-DRAPIDJSON_HAS_STDSTRING=1)

target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDES})
target_link_libraries(${PROJECT_NAME} ${LIBRARIES})

target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME})
target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)



if (UNIX)
else()
    copy_files_to_bin(${ZMQ_DLL})
endif()
