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

    # Package freetype into framework release
    set(freetype_dest system_modules/${PROJECT_NAME}/thirdparty/freetype)
    file(GLOB freetype_docs ${FREETYPE_DIR}/source/docs/*.txt ${FREETYPE_DIR}/source/docs/*.TXT)
    install(FILES ${freetype_docs} DESTINATION ${freetype_dest}/source/docs/)
    install(FILES ${FREETYPE_DIR}/source/README DESTINATION ${freetype_dest}/source/)
    install(DIRECTORY ${FREETYPE_DIR}/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH} DESTINATION ${freetype_dest}/${NAP_THIRDPARTY_PLATFORM_DIR}/)

    if (WIN32)
        # Install for fbxconverter
        install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION tools/buildsystem
            CONFIGURATIONS Release)
    endif()
else()
    set(MODULE_EXTRA_LIBS freetype)
    add_include_to_interface_target(napfont ${FREETYPE_INCLUDE_DIR})

    if(WIN32)
        copy_freetype_dll()
    else()
        file(GLOB rtfreetype_libs ${FREETYPE_DIR}/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH}/lib/libfreetype*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
        install(FILES ${rtfreetype_libs} DESTINATION lib)
    endif()

    install(FILES ${FREETYPE_LICENSE_FILES} DESTINATION licenses/FreeType)
endif()
