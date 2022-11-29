if(NOT TARGET etherdream)
    find_package(etherdream REQUIRED)
endif()

if(NAP_BUILD_CONTEXT MATCHES "source")
    filter_platform_specific_files(SOURCES)
    add_platform_specific_files("${WIN32_SOURCES}" "${MACOS_SOURCES}" "${LINUX_SOURCES}")
    filter_platform_specific_files(HEADERS)
    add_platform_specific_files("${WIN32_SOURCES}" "${MACOS_SOURCES}" "${LINUX_SOURCES}")

    target_include_directories(${PROJECT_NAME} PUBLIC ${ETHERDREAM_INCLUDE_DIR})

    target_link_libraries(${PROJECT_NAME} etherdreamlib)

    if(WIN32)
        # Copy etherdream DLL to build directory on Windows
        if(NOT TARGET etherdreamlib)
            find_package(etherdream REQUIRED)
        endif()
        copy_etherdream_dll()
    endif()

    # Package etherdreamlib into platform release
    install(FILES $<TARGET_FILE:etherdreamlib> DESTINATION thirdparty/etherdream/bin)

    # Package etherdreamlib headers
    install(DIRECTORY ${ETHERDREAM_INCLUDE_DIR}/ DESTINATION thirdparty/etherdream/include)    
    if(WIN32)
        # Package etherdream IMPLIB on Windows
        get_target_property(ETHERDREAM_IMPLIB etherdreamlib IMPORTED_IMPLIB_RELEASE)
        install(FILES ${ETHERDREAM_IMPLIB} DESTINATION thirdparty/etherdream/bin)    
    endif()
else()
    set(MODULE_NAME_EXTRA_LIBS etherdreamlib)

    if(WIN32)
        add_custom_command(TARGET ${PROJECT_NAME}
                           POST_BUILD
                           COMMAND ${CMAKE_COMMAND} 
                                   -E copy
                                   $<TARGET_FILE:etherdreamlib>
                                   $<TARGET_FILE_DIR:${PROJECT_NAME}> 
                           )
    elseif(UNIX)
        # Install etherdream lib into packaged app
        install(FILES $<TARGET_FILE:etherdreamlib> DESTINATION lib)
    endif()
endif()
