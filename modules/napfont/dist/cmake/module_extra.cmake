find_package(freetype REQUIRED)
target_link_libraries(${PROJECT_NAME} freetype)
target_include_directories(${PROJECT_NAME} PUBLIC ${FREETYPE_INCLUDE_DIRS})

if(WIN32)
    # Add post-build step to set copy yoctopuce to bin
    add_custom_command(TARGET ${PROJECT_NAME}
                       POST_BUILD
                       COMMAND ${CMAKE_COMMAND} 
                               -E copy
                               $<TARGET_FILE:freetype>
                               $<TARGET_FILE_DIR:${PROJECT_NAME}> 
                       )
elseif(APPLE)
    file(GLOB RTFREETYPE_LIBS ${THIRDPARTY_DIR}/freetype/lib/libfreetype*${CMAKE_SHARED_LIBRARY_SUFFIX})
      install(FILES ${RTFREETYPE_LIBS} DESTINATION lib)
elseif(UNIX)
    file(GLOB RTFREETYPE_LIBS ${THIRDPARTY_DIR}/freetype/lib/libfree*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
      install(FILES ${RTFREETYPE_LIBS} DESTINATION lib)
endif()
