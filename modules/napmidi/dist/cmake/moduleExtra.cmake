if(WIN32)
    # TODO Add post-build step to set copy rtmidi to bin
elseif(UNIX)
    # Install rtmidi lib into packaged app
    file(GLOB RTMIDI_DYLIBS ${THIRDPARTY_DIR}/rtmidi/lib/librt*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
    install(FILES ${RTMIDI_DYLIBS} DESTINATION lib)
endif()