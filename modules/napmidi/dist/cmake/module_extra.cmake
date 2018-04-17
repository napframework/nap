find_package(rtmidi REQUIRED)
if(NOT WIN32)
    target_link_libraries(${PROJECT_NAME} rtmidi)
endif()
target_include_directories(${PROJECT_NAME} PUBLIC ${RTMIDI_INCLUDE_DIRS})

# Install rtmidi lib into packaged app
if(APPLE)
    install(FILES $<TARGET_FILE:rtmidi> DESTINATION lib)
elseif(UNIX)
    file(GLOB RTMIDI_DYLIBS ${THIRDPARTY_DIR}/rtmidi/lib/librt*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
    install(FILES ${RTMIDI_DYLIBS} DESTINATION lib)
endif()

# Install rtmidi license into packaged project
install(FILES ${THIRDPARTY_DIR}/rtmidi/README.md DESTINATION licenses/rtmidi)