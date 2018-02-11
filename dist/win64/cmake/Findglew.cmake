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


if (WIN32)
    set(GLEW_LIBS ${GLEW_DIR}/bin/glew32.dll)

	add_library(glew SHARED IMPORTED)
	set_target_properties(glew PROPERTIES
	    IMPORTED_CONFIGURATIONS "Debug;Release"
	    IMPORTED_LOCATION_RELEASE ${GLEW_LIBS}
	    IMPORTED_LOCATION_DEBUG ${GLEW_LIBS}
	)

    set_target_properties(glew PROPERTIES
                          IMPORTED_IMPLIB_RELEASE ${GLEW_DIR}/lib/glew32.lib
                          IMPORTED_IMPLIB_DEBUG ${GLEW_DIR}/lib/glew32.lib
                          )

    # Copy over DLL post-build
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:glew> $<TARGET_FILE_DIR:${PROJECT_NAME}>/
    )
endif()