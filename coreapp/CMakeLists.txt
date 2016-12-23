project(CoreApp)

file(GLOB sources src/*.cpp)
include_directories(src)

source_group(resources resources/*.*)

add_definitions(-DAPP_NAME="${PROJECT_NAME}")

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
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_LIST_DIR}/resources" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources"
        COMMENT "Copy ${CMAKE_CURRENT_LIST_DIR}/resources to ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources")