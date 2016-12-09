project(unittest)

include_directories(src)
file(GLOB_RECURSE SOURCES src/ *.cpp *.h)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} napcore mod_napjsonserializer)

set(RESOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/resources)



function(copy_files SRCDIR DSTDIR)
    file(GLOB_RECURSE RESOURCES ${SRCDIR}/*.*)
    foreach(RES_FILE ${RESOURCES})
        get_filename_component(NAME ${RES_FILE} NAME)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E
                ARGS copy "${RES_FILE}" "${DSTDIR}/${NAME}"
                COMMENT "Copy '${RES_FILE}' to '${DSTDIR}/${NAME}'"
                DEPENDS ${RES_FILE}
                VERBATIM
                )
        string(REPLACE "/" "_" TARGET_NAME "CopyFile_${RES_FILE}")
        add_custom_target(${TARGET_NAME} ALL
                DEPENDS ${RES_FILE})
        add_dependencies(${PROJECT_NAME} ${TARGET_NAME})
    endforeach()
endfunction(copy_files)

copy_files("${CMAKE_CURRENT_LIST_DIR}/resources" "${BIN_DIR}/resources")


