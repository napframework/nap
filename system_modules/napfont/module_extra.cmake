find_package(freetype REQUIRED)
target_include_directories(${PROJECT_NAME} PUBLIC ${FREETYPE_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} freetype)

if (WIN32)
    # Install for fbxconverter
    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION tools/buildsystem
        CONFIGURATIONS Release)
endif()

add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:freetype>
        ${LIB_DIR})
