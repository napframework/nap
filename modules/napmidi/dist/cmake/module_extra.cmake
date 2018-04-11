# Install rtmidi lib into packaged app
if(APPLE)
    install(FILES ${THIRDPARTY_DIR}/rtmidi/lib/librtmidi.4.dylib DESTINATION lib)
elseif(UNIX)
    file(GLOB RTMIDI_DYLIBS ${THIRDPARTY_DIR}/rtmidi/lib/librt*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
    install(FILES ${RTMIDI_DYLIBS} DESTINATION lib)
endif()

# Install rtmidi license into packaged project
install(FILES ${THIRDPARTY_DIR}/rtmidi/README.md DESTINATION licenses/rtmidi)
