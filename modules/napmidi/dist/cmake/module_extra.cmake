include(${NAP_ROOT}/cmake/dist_shared_crossplatform.cmake)

if(NOT TARGET rtmidi)
    find_package(rtmidi REQUIRED)
endif()
set(MODULE_NAME_EXTRA_LIBS rtmidi)
if(WIN32)
     set(MODULE_NAME_EXTRA_LIBS "${MODULE_NAME_EXTRA_LIBS}" winmm)
endif()
add_include_to_interface_target(mod_napmidi ${RTMIDI_INCLUDE_DIRS})

if(NOT TARGET moodycamel)
    find_package(moodycamel REQUIRED)
endif()
add_include_to_interface_target(mod_napmidi ${MOODYCAMEL_INCLUDE_DIRS})


# Install rtmidi lib into packaged app
if(APPLE)
    install(FILES $<TARGET_FILE:rtmidi> DESTINATION lib)
elseif(UNIX)
    file(GLOB RTMIDI_DYLIBS ${THIRDPARTY_DIR}/rtmidi/lib/librt*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
    install(FILES ${RTMIDI_DYLIBS} DESTINATION lib)
endif()

# Install rtmidi license into packaged project
install(FILES ${THIRDPARTY_DIR}/rtmidi/README.md DESTINATION licenses/rtmidi)