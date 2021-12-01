# - Try to find PORTAUDIO
# Once done this will define
# PORTAUDIO_FOUND - System has PORTAUDIO
# PORTAUDIO_INCLUDE_DIRS - The PORTAUDIO include directories
# PORTAUDIO_LIBRARIES - The libraries needed to use PORTAUDIO
# PORTAUDIO_DEFINITIONS - Compiler switches required for using PORTAUDIO


include(${CMAKE_CURRENT_LIST_DIR}/targetarch.cmake)
target_architecture(ARCH)

if(NOT ANDROID)
    find_path(PORTAUDIO_DIR 
              NAMES portaudio.h
              HINTS ${THIRDPARTY_DIR}/portaudio/msvc/x86_64/include
              ${THIRDPARTY_DIR}/portaudio/macos/x86_64/include
              ${THIRDPARTY_DIR}/portaudio/linux/${ARCH}/include
              ${CMAKE_CURRENT_LIST_DIR}/../../portaudio
              )
endif()

if(WIN32)
    set(PORTAUDIO_INCLUDE_DIR ${PORTAUDIO_DIR}/msvc/x86_64/include)
    set(PORTAUDIO_LIB_DIR ${PORTAUDIO_DIR}/msvc/x86_64/lib)
    set(PORTAUDIO_LIBRARIES ${PORTAUDIO_LIB_DIR}/portaudio_x64.lib)
    set(PORTAUDIO_LIBS_RELEASE_DLL ${PORTAUDIO_LIB_DIR}/portaudio_x64.dll)
elseif(APPLE)
    set(PORTAUDIO_INCLUDE_DIR ${PORTAUDIO_DIR}/macos/x86_64/include)
    set(PORTAUDIO_LIB_DIR /${PORTAUDIO_DIR}/macos/x86_64/lib)
    set(PORTAUDIO_LIBS_RELEASE_DLL ${PORTAUDIO_LIB_DIR}/libportaudio.2.dylib)
    set(PORTAUDIO_LIBRARIES ${PORTAUDIO_LIBS_RELEASE_DLL})
elseif(ANDROID)
    set(PORTAUDIO_DIR ${THIRDPARTY_DIR}/portaudio_opensles)
    set(PORTAUDIO_LIB_DIR ${PORTAUDIO_DIR}/android/lib/Release/${ANDROID_ABI})
    set(PORTAUDIO_LIBS_RELEASE_DLL ${PORTAUDIO_LIB_DIR}/libportaudio.so)
    set(PORTAUDIO_LIBRARIES ${PORTAUDIO_LIBS_RELEASE_DLL})
    # Check for our header to do some basic verification (that we're missing by not doing find_path above for 
    # Android)
    find_path(PORTAUDIO_INCLUDE_DIR portaudio.h
              HINTS
              ${PORTAUDIO_DIR}/include
              )
else()
    set(PORTAUDIO_INCLUDE_DIR ${PORTAUDIO_DIR}/linux/${ARCH}/include)
    set(PORTAUDIO_LIB_DIR ${PORTAUDIO_DIR}/linux/${ARCH}/lib)
    set(PORTAUDIO_LIBS_RELEASE_DLL ${PORTAUDIO_LIB_DIR}/libportaudio.so)
    set(PORTAUDIO_LIBRARIES ${PORTAUDIO_LIBS_RELEASE_DLL})
endif()


include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set PORTAUDIO_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(portaudio REQUIRED_VARS PORTAUDIO_LIBRARIES PORTAUDIO_INCLUDE_DIR PORTAUDIO_DIR)


add_library(portaudio SHARED IMPORTED)
set_target_properties(portaudio PROPERTIES
                      IMPORTED_CONFIGURATIONS "Debug;Release;MinSizeRel;RelWithDebInfo"
                      IMPORTED_LOCATION_RELEASE ${PORTAUDIO_LIBS_RELEASE_DLL}
                      IMPORTED_LOCATION_DEBUG ${PORTAUDIO_LIBS_RELEASE_DLL}
                      IMPORTED_LOCATION_MINSIZEREL ${PORTAUDIO_LIBS_RELEASE_DLL}
                      IMPORTED_LOCATION_RELWITHDEBINFO ${PORTAUDIO_LIBS_RELEASE_DLL}
                      )

if(WIN32)
    set_target_properties(portaudio PROPERTIES
                          IMPORTED_IMPLIB ${PORTAUDIO_LIBRARIES}
                          )      
endif()  

# Copy the portaudio dynamic linked lib into the build directory
macro(copy_portaudio_lib)
    if(WIN32)
        add_custom_command(TARGET ${PROJECT_NAME}
                           POST_BUILD
                           COMMAND ${CMAKE_COMMAND} -E copy_if_different
                           $<TARGET_FILE:portaudio>
                           "$<TARGET_PROPERTY:${PROJECT_NAME},RUNTIME_OUTPUT_DIRECTORY_$<UPPER_CASE:$<CONFIG>>>"
                           )
    endif()
endmacro()
