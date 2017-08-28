# - Try to find PORTAUDIO
# Once done this will define
# PORTAUDIO_FOUND - System has PORTAUDIO
# PORTAUDIO_INCLUDE_DIRS - The PORTAUDIO include directories
# PORTAUDIO_LIBRARIES - The libraries needed to use PORTAUDIO
# PORTAUDIO_DEFINITIONS - Compiler switches required for using PORTAUDIO



if(WIN32)
    include(${CMAKE_CURRENT_LIST_DIR}/targetarch.cmake)
    target_architecture(ARCH)

    find_path(PORTAUDIO_DIR include/portaudio.h
            HINTS
            ${CMAKE_CURRENT_LIST_DIR}/../../thirdparty/portaudio
            ${CMAKE_CURRENT_LIST_DIR}/../../portaudio
            )


    set(PORTAUDIO_LIB_DIR ${PORTAUDIO_DIR}/msvc64)
    set(PORTAUDIO_LIBRARIES ${PORTAUDIO_LIB_DIR}/portaudio_x64.lib)

    set(PORTAUDIO_INCLUDE_DIR ${PORTAUDIO_DIR}/include)

    include(FindPackageHandleStandardArgs)
    # handle the QUIETLY and REQUIRED arguments and set PORTAUDIO_FOUND to TRUE
    # if all listed variables are TRUE
    find_package_handle_standard_args(PORTAUDIO DEFAULT_MSG PORTAUDIO_LIBRARIES PORTAUDIO_INCLUDE_DIR)
elseif(APPLE)
    include(${CMAKE_CURRENT_LIST_DIR}/targetarch.cmake)
    target_architecture(ARCH)

    find_path(PORTAUDIO_DIR include/portaudio.h
            HINTS
            ${CMAKE_CURRENT_LIST_DIR}/../../thirdparty/portaudio
            ${CMAKE_CURRENT_LIST_DIR}/../../portaudio
            )

    set(PORTAUDIO_LIB_DIR /usr/local/lib)
    set(PORTAUDIO_LIBRARIES ${PORTAUDIO_LIB_DIR}/libportaudio.a)

    set(PORTAUDIO_INCLUDE_DIR ${PORTAUDIO_DIR}/include)

    include(FindPackageHandleStandardArgs)
    # handle the QUIETLY and REQUIRED arguments and set PORTAUDIO_FOUND to TRUE
    # if all listed variables are TRUE
    find_package_handle_standard_args(PORTAUDIO DEFAULT_MSG PORTAUDIO_LIBRARIES PORTAUDIO_INCLUDE_DIR)
endif()



