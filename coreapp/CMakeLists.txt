project(CoreApp)

file(GLOB sources src/*.cpp)
include_directories(src)

add_definitions(-DAPP_NAME="${PROJECT_NAME}")
add_definitions(-DRAPIDJSON_HAS_STDSTRING=1)

add_executable(${PROJECT_NAME} ${sources})

set(_LIBS napcore mod_napscriptserver)

if(MSVC)
else()
    list(APPEND _LIBS pthread)
endif()

target_link_libraries(${PROJECT_NAME} ${_LIBS})

# Copy the binary resources to the bin dir so the executable can load them.
add_custom_command(TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_LIST_DIR}/resources" "${BIN_DIR}/resources"
        COMMENT "Copy ${CMAKE_CURRENT_LIST_DIR}/resources to ${BIN_DIR}/resources")