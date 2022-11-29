if(NOT TARGET freetype)
    find_package(freetype REQUIRED)
endif()

if(NAP_BUILD_CONTEXT MATCHES "source")
    target_include_directories(${PROJECT_NAME} PUBLIC ${FREETYPE_INCLUDE_DIR})

    target_link_libraries(${PROJECT_NAME} freetype)

    # Copy freetype DLL
    if (WIN32)
        copy_freetype_dll()
    endif()

    # Package freetype into platform release
    install(FILES ${FREETYPE_DIST_FILES} DESTINATION thirdparty/freetype)

    install(DIRECTORY ${FREETYPE_INCLUDE_DIR}/ DESTINATION thirdparty/freetype/include
            FILES_MATCHING PATTERN "*.h")

    if(MSVC)
        install(FILES ${FREETYPE_LIBS_RELEASE} ${FREETYPE_DLL_RELEASE} DESTINATION thirdparty/freetype/bin/release)
        install(FILES ${FREETYPE_LIBS_DEBUG} ${FREETYPE_DLL_DEBUG} DESTINATION thirdparty/freetype/bin/debug)
    elseif(APPLE)
        file(GLOB RTFREETYPE_LIBS ${FREETYPE_LIBS_DIR}/libfreetype*${CMAKE_SHARED_LIBRARY_SUFFIX})
        install(FILES ${RTFREETYPE_LIBS}
                DESTINATION thirdparty/freetype/lib)
    elseif(UNIX)
        file(GLOB RTFREETYPE_LIBS ${FREETYPE_LIBS_DIR}/libfreetype${CMAKE_SHARED_LIBRARY_SUFFIX}*)
        install(FILES ${RTFREETYPE_LIBS}
                DESTINATION thirdparty/freetype/lib)
    endif()

    if (WIN32)
        # Install for fbxconverter
        install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION tools/platform
            CONFIGURATIONS Release)
    endif()
else()
    set(MODULE_NAME_EXTRA_LIBS freetype)
    add_include_to_interface_target(mod_napfont ${FREETYPE_INCLUDE_DIRS})

    if(WIN32)
        copy_freetype_dll()
    elseif(APPLE)
        file(GLOB RTFREETYPE_LIBS ${THIRDPARTY_DIR}/freetype/lib/libfreetype*${CMAKE_SHARED_LIBRARY_SUFFIX})
          install(FILES ${RTFREETYPE_LIBS} DESTINATION lib)
    elseif(UNIX)
        file(GLOB RTFREETYPE_LIBS ${THIRDPARTY_DIR}/freetype/lib/libfree*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
          install(FILES ${RTFREETYPE_LIBS} DESTINATION lib)
    endif()
endif()
