if(NOT TARGET rtmidi)
    find_package(rtmidi REQUIRED)
endif()

set(rtmidi_dest_dir system_modules/napmidi/thirdparty/rtmidi)
if(NAP_BUILD_CONTEXT MATCHES "source")
    target_include_directories(${PROJECT_NAME} PUBLIC ${RTMIDI_INCLUDE_DIR})

    target_link_libraries(${PROJECT_NAME} debug ${RTMIDI_LIBRARIES_DEBUG} optimized ${RTMIDI_LIBRARIES_RELEASE})

    # Package rtmidi into platform release
    if(MSVC)
        file(GLOB RTMIDI_STATIC_LIBS ${RTMIDI_LIBRARY_DIR}/rtmidi*${CMAKE_STATIC_LIBRARY_SUFFIX})
        install(FILES ${RTMIDI_STATIC_LIBS}
                DESTINATION ${rtmidi_dest_dir}/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH}/lib)
    elseif(APPLE)
        file(GLOB RTMIDI_DYLIBS ${RTMIDI_LIBRARY_DIR}/librtmidi*${CMAKE_SHARED_LIBRARY_SUFFIX})
        install(FILES ${RTMIDI_DYLIBS}
                DESTINATION ${rtmidi_dest_dir}/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH}/lib)
    elseif(UNIX)
        file(GLOB RTMIDI_DYLIBS ${RTMIDI_LIBRARY_DIR}/librtmidi${CMAKE_SHARED_LIBRARY_SUFFIX}*)
        install(FILES ${RTMIDI_DYLIBS}
                DESTINATION ${rtmidi_dest_dir}/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH}/lib)
    endif()
    install(DIRECTORY ${RTMIDI_INCLUDE_DIR} DESTINATION ${rtmidi_dest_dir}/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH}/include)
    install(FILES ${RTMIDI_DIR}/source/README.md DESTINATION ${rtmidi_dest_dir}/source)
else()
    # As with oscpack this is a minor regression, could be solved usig the old framework release style find module
    set(MODULE_NAME_EXTRA_LIBS ${RTMIDI_LIBRARIES_RELEASE})
    if(WIN32)
         set(MODULE_NAME_EXTRA_LIBS "${MODULE_NAME_EXTRA_LIBS}" winmm)
    endif()
    add_include_to_interface_target(napmidi ${RTMIDI_INCLUDE_DIR})

    if(NOT TARGET moodycamel)
        find_package(moodycamel REQUIRED)
    endif()
    add_include_to_interface_target(napmidi ${MOODYCAMEL_INCLUDE_DIRS})

    # Install rtmidi lib into packaged app
    if(APPLE)
        install(FILES ${RTMIDI_LIBRARIES_RELEASE} DESTINATION lib)
    elseif(UNIX)
        file(GLOB RTMIDI_DYLIBS ${NAP_ROOT}/${rtmidi_dest_dir}/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH}/lib/librt*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
        install(FILES ${RTMIDI_DYLIBS} DESTINATION lib)
    endif()

    # Install rtmidi license into packaged project
    install(FILES ${RTMIDI_LICENSE_FILES} DESTINATION licenses/RtMidi)
endif()
