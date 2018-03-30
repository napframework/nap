if(UNIX AND NOT APPLE)
    # Install rtmidi lib into packaged app
    file(GLOB RTMIDI_DYLIBS ${THIRDPARTY_DIR}/rtmidi/lib/librt*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
    install(FILES ${RTMIDI_DYLIBS} DESTINATION lib)
endif()

# Install rtmidi into packaged project
install(FILES ${THIRDPARTY_DIR}/rtmidi/README.md DESTINATION licenses/rtmidi)
