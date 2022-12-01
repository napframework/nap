if(NOT TARGET yoctopuce)
    find_package(yoctopuce REQUIRED)
endif()

if(NAP_BUILD_CONTEXT MATCHES "source")
    target_include_directories(${PROJECT_NAME} PUBLIC src ${YOCTO_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} yoctopuce)

    if(WIN32)
      copy_yoctopuce_dll()
    endif()

    # Package yoctopuce into platform release
    install(FILES ${YOCTO_DIST_FILES} DESTINATION thirdparty/yoctopuce)
    install(DIRECTORY ${YOCTO_INCLUDE_DIR}/ DESTINATION thirdparty/yoctopuce/include
            FILES_MATCHING PATTERN "*.h")

    if(WIN32)
        install(FILES ${YOCTO_LIBS_RELEASE} ${YOCTO_RELEASE_DLL} DESTINATION thirdparty/yoctopuce/bin/release)
        install(FILES ${YOCTO_LIBS_DEBUG} ${YOCTO_DEBUG_DLL} DESTINATION thirdparty/yoctopuce/bin/debug)
    elseif(APPLE)
        install(FILES ${YOCTO_LIBS_RELEASE} DESTINATION thirdparty/yoctopuce/bin)

        # Ensure our yoctopuce install name id is set properly, this is really for intall into packaging
        add_custom_command(TARGET ${PROJECT_NAME}
                           PRE_BUILD
                           COMMAND ${CMAKE_INSTALL_NAME_TOOL} -id @rpath/libyocto.dylib ${YOCTO_LIBS_RELEASE}
                           COMMENT "Setting install name for yoctopuce")

        foreach(build_type Release Debug)
            install(CODE "execute_process(COMMAND ${CMAKE_INSTALL_NAME_TOOL}
                                                  -add_rpath
                                                  @loader_path/../../../../thirdparty/yoctopuce/bin
                                                  ${CMAKE_INSTALL_PREFIX}/modules/napyoctopuce/lib/${build_type}/napyoctopuce.dylib
                                          ERROR_QUIET
                                          )")
        endforeach()    
    else()
        install(FILES ${YOCTO_LIBS_RELEASE} DESTINATION thirdparty/yoctopuce/bin)
    endif()
else()
    set(MODULE_NAME_EXTRA_LIBS yoctopuce)

    add_include_to_interface_target(napyoctopuce ${YOCTOPUCE_INCLUDE_DIRS})

    if(WIN32)
        # Add post-build step to set copy yoctopuce to bin
        add_custom_command(TARGET ${PROJECT_NAME}
                           POST_BUILD
                           COMMAND ${CMAKE_COMMAND} 
                                   -E copy
                                   $<TARGET_FILE:yoctopuce>
                                   $<TARGET_FILE_DIR:${PROJECT_NAME}> 
                           )
    elseif(UNIX)
        # Install yoctopuce lib into packaged app
        install(FILES $<TARGET_FILE:yoctopuce> DESTINATION lib)
    endif()

    # TODO Install yoctopuce license with packaged app if we find one
endif()
