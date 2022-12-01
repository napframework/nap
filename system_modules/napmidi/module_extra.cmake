if(NOT TARGET rtmidi)
    find_package(rtmidi REQUIRED)
endif()

if(NAP_BUILD_CONTEXT MATCHES "source")
    target_include_directories(${PROJECT_NAME} PUBLIC ${RTMIDI_INCLUDE_DIR})

    target_link_libraries(${PROJECT_NAME} debug ${RTMIDI_LIBRARIES_DEBUG} optimized ${RTMIDI_LIBRARIES_RELEASE} )

    # Package rtmidi into platform release
    install(FILES ${RTMIDI_DIST_FILES} DESTINATION thirdparty/rtmidi)
    install(DIRECTORY ${RTMIDI_INCLUDE_DIR}/ DESTINATION thirdparty/rtmidi/include)
    if(MSVC)
        file(GLOB RTMIDI_STATIC_LIBS ${RTMIDI_LIBRARY_DIR}/rtmidi*${CMAKE_STATIC_LIBRARY_SUFFIX})
        install(FILES ${RTMIDI_STATIC_LIBS}
                DESTINATION thirdparty/rtmidi/bin)
    elseif(APPLE)
        file(GLOB RTMIDI_DYLIBS ${RTMIDI_LIBRARY_DIR}/librtmidi*${CMAKE_SHARED_LIBRARY_SUFFIX})
        install(FILES ${RTMIDI_DYLIBS}
                DESTINATION thirdparty/rtmidi/lib)
    elseif(UNIX)
        file(GLOB RTMIDI_DYLIBS ${RTMIDI_LIBRARY_DIR}/librtmidi${CMAKE_SHARED_LIBRARY_SUFFIX}*)
        install(FILES ${RTMIDI_DYLIBS}
                DESTINATION thirdparty/rtmidi/lib)
    endif()
else()
    set(MODULE_NAME_EXTRA_LIBS rtmidi)
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
        install(FILES $<TARGET_FILE:rtmidi> DESTINATION lib)
    elseif(UNIX)
        file(GLOB RTMIDI_DYLIBS ${THIRDPARTY_DIR}/rtmidi/lib/librt*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
        install(FILES ${RTMIDI_DYLIBS} DESTINATION lib)
    endif()

    # Install rtmidi license into packaged project
    install(FILES ${THIRDPARTY_DIR}/rtmidi/README.md DESTINATION licenses/rtmidi)
endif()
