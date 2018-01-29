# - Try to find PORTAUDIO
# Once done this will define
# PORTAUDIO_FOUND - System has PORTAUDIO
# PORTAUDIO_INCLUDE_DIRS - The PORTAUDIO include directories
# PORTAUDIO_LIBRARIES - The libraries needed to use PORTAUDIO
# PORTAUDIO_DEFINITIONS - Compiler switches required for using PORTAUDIO


include(${CMAKE_CURRENT_LIST_DIR}/targetarch.cmake)
target_architecture(ARCH)

find_path(PORTAUDIO_DIR include/portaudio.h
          HINTS
          ${THIRDPARTY_DIR}/portaudio
          ${CMAKE_CURRENT_LIST_DIR}/../../portaudio
          )


if(WIN32)
    set(PORTAUDIO_LIB_DIR ${PORTAUDIO_DIR}/msvc64)
    set(PORTAUDIO_LIBRARIES ${PORTAUDIO_LIB_DIR}/portaudio_x64.lib)
    set(PORTAUDIO_LIBS_RELEASE_DLL ${PORTAUDIO_LIB_DIR}/portaudio_x64.dll)
elseif(APPLE)
    set(PORTAUDIO_LIB_DIR /${PORTAUDIO_DIR}/xcode)
    set(PORTAUDIO_LIBRARIES ${PORTAUDIO_LIB_DIR}/libportaudio.a)
else()
    if(${ARCH} STREQUAL "armv6")
        set(PORTAUDIO_LIB_DIR ${PORTAUDIO_DIR}/linux/arm)
        set(PORTAUDIO_LIBRARIES ${PORTAUDIO_LIB_DIR}/libportaudio.so)
    else()
        set(PORTAUDIO_LIB_DIR ${PORTAUDIO_DIR}/linux)
        set(PORTAUDIO_LIBRARIES ${PORTAUDIO_LIB_DIR}/libportaudio.so)
    endif()
endif()

set(PORTAUDIO_INCLUDE_DIR ${PORTAUDIO_DIR}/include)

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set PORTAUDIO_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(PORTAUDIO DEFAULT_MSG PORTAUDIO_LIBRARIES PORTAUDIO_INCLUDE_DIR)


# Copy the portaudio dynamic linked lib into the build directory
macro(copy_portaudio_lib)
    if(WIN32)
        add_library(portaudiolib SHARED IMPORTED)
        set_target_properties(portaudiolib PROPERTIES
                              IMPORTED_CONFIGURATIONS "Debug;Release;MinSizeRel;RelWithDebInfo"
                              IMPORTED_LOCATION_RELEASE ${PORTAUDIO_LIBS_RELEASE_DLL}
                              IMPORTED_LOCATION_DEBUG ${PORTAUDIO_LIBS_RELEASE_DLL}
                              IMPORTED_LOCATION_MINSIZEREL ${PORTAUDIO_LIBS_RELEASE_DLL}
                              IMPORTED_LOCATION_RELWITHDEBINFO ${PORTAUDIO_LIBS_RELEASE_DLL}
                              )

        add_custom_command(TARGET ${PROJECT_NAME}
                           POST_BUILD
                           COMMAND ${CMAKE_COMMAND} -E
                           copy $<TARGET_FILE:portaudiolib>
                           $<TARGET_FILE_DIR:${PROJECT_NAME}>/$<TARGET_FILE_NAME:portaudiolib>
                           )
    endif()
endmacro()
