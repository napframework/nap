if(WIN32)
    find_path(
            ETHERDREAM_DIR
            NAMES msvc/bin/EtherDream.dll
            HINTS ${THIRDPARTY_DIR}/etherdream
    )
    set(ETHERDREAM_LIBS_DIR ${ETHERDREAM_DIR}/msvc/bin/)
    set(ETHERDREAM_LIBS ${ETHERDREAM_LIBS_DIR}/EtherDream.lib)
    set(ETHERDREAM_LIBS_RELEASE_DLL ${ETHERDREAM_LIBS_DIR}/EtherDream.dll)
elseif(APPLE)
    find_path(ETHERDREAM_DIR
              NAMES osx/bin/libEtherDream.dylib
              HINTS ${THIRDPARTY_DIR}/etherdream
              )
    set(ETHERDREAM_LIBS_DIR ${ETHERDREAM_DIR}/osx/bin/)
    set(ETHERDREAM_LIBS ${ETHERDREAM_LIBS_DIR}/libEtherDream.dylib)
    set(ETHERDREAM_LIBS_RELEASE_DLL ${ETHERDREAM_LIBS_DIR}/libEtherDream.dylib)
else()
    find_path(ETHERDREAM_DIR
              NAMES linux/bin/libetherdream.so
              HINTS ${THIRDPARTY_DIR}/etherdream
              )
    set(ETHERDREAM_LIBS_DIR ${ETHERDREAM_DIR}/linux/bin)
    set(ETHERDREAM_LIBS ${ETHERDREAM_LIBS_DIR}/libetherdream.so)
    set(ETHERDREAM_LIBS_RELEASE_DLL ${ETHERDREAM_LIBS_DIR}/libetherdream.so)
endif()

mark_as_advanced(ETHERDREAM_LIBS_DIR)

add_library(etherdreamlib SHARED IMPORTED)
set_target_properties(etherdreamlib PROPERTIES
                      IMPORTED_CONFIGURATIONS "Debug;Release"
                      IMPORTED_LOCATION_RELEASE ${ETHERDREAM_LIBS_RELEASE_DLL}
                      IMPORTED_LOCATION_DEBUG ${ETHERDREAM_LIBS_RELEASE_DLL}
                      )

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(etherdream REQUIRED_VARS ETHERDREAM_DIR ETHERDREAM_LIBS ETHERDREAM_LIBS_DIR)

# Copy the etherdream dynamic linked lib into the build directory
macro(copy_etherdream_dll)
    add_custom_command(
            TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:etherdreamlib> $<TARGET_FILE_DIR:${PROJECT_NAME}>/$<TARGET_FILE_NAME:etherdreamlib>
    )
endmacro()