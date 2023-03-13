if(NOT TARGET yoctopuce)
    find_package(yoctopuce REQUIRED)
endif()

if(NAP_BUILD_CONTEXT MATCHES "source")
    target_include_directories(${PROJECT_NAME} PUBLIC src ${YOCTOPUCE_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} yoctopuce)

    if(WIN32)
      copy_yoctopuce_dll()
    endif()

    # Package yoctopuce into platform release
    set(yoctopuce_dest system_modules/${PROJECT_NAME}/thirdparty/yoctopuce)
    install(FILES ${YOCTOPUCE_DIR}/README.txt DESTINATION ${yoctopuce_dest}/)
    install(DIRECTORY ${YOCTOPUCE_INCLUDE_DIR}/ DESTINATION ${yoctopuce_dest}/source/Sources
            FILES_MATCHING PATTERN "*.h")
    install(DIRECTORY ${YOCTOPUCE_DIR}/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH} DESTINATION ${yoctopuce_dest}/${NAP_THIRDPARTY_PLATFORM_DIR}/)

    if(APPLE)
        # Ensure our yoctopuce install name id is set properly, this is really for intall into packaging
        add_custom_command(TARGET ${PROJECT_NAME}
                           PRE_BUILD
                           COMMAND ${CMAKE_INSTALL_NAME_TOOL} -id @rpath/libyocto.dylib ${YOCTOPUCE_LIBS_RELEASE}
                           COMMENT "Setting install name for yoctopuce")

        foreach(build_type Release Debug)
            install(CODE "execute_process(COMMAND ${CMAKE_INSTALL_NAME_TOOL}
                                                  -add_rpath
                                                  @loader_path/../../../../thirdparty/yoctopuce/bin
                                                  ${CMAKE_INSTALL_PREFIX}/system_modules/napyoctopuce/lib/${build_type}/napyoctopuce.dylib
                                          ERROR_QUIET
                                          )")
        endforeach()
    endif()
else()
    set(MODULE_EXTRA_LIBS yoctopuce)

    add_include_to_interface_target(napyoctopuce ${YOCTOPUCE_INCLUDE_DIRS})

    # Install yoctopuce lib into packaged app
    if(WIN32)
        # Add post-build step to set copy yoctopuce to bin
        add_custom_command(TARGET ${PROJECT_NAME}
                           POST_BUILD
                           COMMAND ${CMAKE_COMMAND}
                                   -E copy
                                   $<TARGET_FILE:yoctopuce>
                                   $<TARGET_FILE_DIR:${PROJECT_NAME}>
                           )
    elseif(APPLE)
        install(FILES $<TARGET_FILE:yoctopuce> DESTINATION lib)
    else()
        file(GLOB yoctolibs ${YOCTOPUCE_DIR}/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH}/*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
        install(FILES ${yoctolibs} DESTINATION lib)
    endif()

    install(FILES ${YOCTOPUCE_LICENSE_FILES} DESTINATION licenses/yoctopuce)
endif()
