find_package(rtmidi REQUIRED)
target_link_libraries(${PROJECT_NAME} rtmidi)
target_include_directories(${PROJECT_NAME} PUBLIC ${RTMIDI_INCLUDE_DIRS})

if(UNIX AND NOT APPLE)
    # Install rtmidi lib into packaged app
    file(GLOB RTMIDI_DYLIBS ${THIRDPARTY_DIR}/rtmidi/lib/librt*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
    install(FILES ${RTMIDI_DYLIBS} DESTINATION lib)
endif()