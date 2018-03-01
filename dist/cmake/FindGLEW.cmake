find_path(
    GLEW_DIR
    NAMES include/GL/glew.h
    HINTS
    ${THIRDPARTY_DIR}/glew/
)

set(GLEW_INCLUDE_DIRS ${GLEW_DIR}/include/)

mark_as_advanced(GLEW_INCLUDE_DIRS)

# TODO later: Fix CMake approach and use config-style package files

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(glew REQUIRED_VARS GLEW_DIR GLEW_INCLUDE_DIRS)

if(UNIX)
    set(GLEW_LIBS ${GLEW_DIR}/lib/libGLEW${CMAKE_SHARED_LIBRARY_SUFFIX})
elseif (WIN32)
    set(GLEW_LIBS ${GLEW_DIR}/bin/glew32.dll)
endif()

add_library(GLEW SHARED IMPORTED)
set_target_properties(GLEW PROPERTIES
    IMPORTED_CONFIGURATIONS "Debug;Release"
    IMPORTED_LOCATION_RELEASE ${GLEW_LIBS}
    IMPORTED_LOCATION_DEBUG ${GLEW_LIBS}
)

if (WIN32)
    set_target_properties(GLEW PROPERTIES
                          IMPORTED_IMPLIB ${GLEW_DIR}/lib/glew32.lib
                          )

    # Copy over DLL post-build
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:GLEW> $<TARGET_FILE_DIR:${PROJECT_NAME}>/
    )
endif()