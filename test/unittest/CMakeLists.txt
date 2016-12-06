project(unittest)

include_directories(src)
file(GLOB_RECURSE SOURCES src/ *.cpp *.h)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} napcore mod_napjsonserializer)

set(RESOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/resources)

file(GLOB_RECURSE RESOURCES ${CMAKE_CURRENT_LIST_DIR}/resources)
foreach(RES_FILE ${RESOURCES})
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E
            ARGS copy "${RES_FILE}" "${BIN_DIR}/resources"
            COMMENT "Copy '${RES_FILE}' to '${BIN_DIR}/resources'"
            MAIN_DEPENDENCY ${RES_FILE}
            VERBATIM
            )
endforeach()
