if(WIN32)
    find_path(
            ETHERDREAM_DIR
            NAMES msvc/include/j4cDAC.h
            HINTS ${THIRDPARTY_DIR}/etherdream
    )
    set(ETHERDREAM_INCLUDE_DIRS ${ETHERDREAM_DIR}/msvc/include)
    set(ETHERDREAM_LIBS ${ETHERDREAM_DIR}/msvc/bin/Release/EtherDream.lib)
    set(ETHERDREAM_LIBS_DIR ${ETHERDREAM_DIR}/msvc/bin/Release)
    set(ETHERDREAM_LIBS_RELEASE_DLL ${ETHERDREAM_LIBS_DIR}/EtherDream.dll)
elseif(APPLE)
    find_path(ETHERDREAM_DIR
              NAMES osx/include/etherdream.h
              HINTS ${THIRDPARTY_DIR}/etherdream
              )
    set(ETHERDREAM_INCLUDE_DIRS ${ETHERDREAM_DIR}/osx/include)
    set(ETHERDREAM_LIBS ${ETHERDREAM_DIR}/osx/bin/Release/libEtherDream.dylib)
    set(ETHERDREAM_LIBS_DIR ${ETHERDREAM_DIR}/osx/bin/Release)
    set(ETHERDREAM_LIBS_RELEASE_DLL ${ETHERDREAM_LIBS_DIR}/libEtherDream.dylib)
else()
    find_path(ETHERDREAM_DIR
              NAMES linux/include/etherdream.h
              HINTS ${THIRDPARTY_DIR}/etherdream
              )
    set(ETHERDREAM_INCLUDE_DIRS ${ETHERDREAM_DIR}/linux/include)
    set(ETHERDREAM_LIBS ${ETHERDREAM_DIR}/linux/bin/libetherdream.so)
    set(ETHERDREAM_LIBS_DIR ${ETHERDREAM_DIR}/linux/bin)
    set(ETHERDREAM_LIBS_RELEASE_DLL ${ETHERDREAM_LIBS_DIR}/libetherdream.so)
endif()

mark_as_advanced(ETHERDREAM_INCLUDE_DIRS)
mark_as_advanced(ETHERDREAM_LIBS_DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(etherdream REQUIRED_VARS ETHERDREAM_INCLUDE_DIRS ETHERDREAM_LIBS ETHERDREAM_LIBS_DIR)

# Copy the etherdream dynamic linked lib into the build directory
macro(copy_etherdream_dll)
    add_library(etherdreamlib SHARED IMPORTED)
    set_target_properties(etherdreamlib PROPERTIES
                          IMPORTED_CONFIGURATIONS "Debug;Release;MinSizeRel;RelWithDebInfo"
                          IMPORTED_LOCATION_RELEASE ${ETHERDREAM_LIBS_RELEASE_DLL}
                          IMPORTED_LOCATION_DEBUG ${ETHERDREAM_LIBS_RELEASE_DLL}
                          IMPORTED_LOCATION_MINSIZEREL ${ETHERDREAM_LIBS_RELEASE_DLL}
                          IMPORTED_LOCATION_RELWITHDEBINFO ${ETHERDREAM_LIBS_RELEASE_DLL}
                          )

    add_custom_command(
            TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:etherdreamlib> $<TARGET_FILE_DIR:${PROJECT_NAME}>/$<TARGET_FILE_NAME:etherdreamlib>
    )
endmacro()
