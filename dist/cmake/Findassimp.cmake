if (WIN32)
    find_path(
        ASSIMP_LIBS_DIR
        NAMES assimp-vc140-mt.dll
        HINTS
        ${CMAKE_CURRENT_LIST_DIR}/../thirdparty/assimp/bin/
    )

    set(ASSIMP_LIBS ${ASSIMP_LIBS_DIR}/assimp-vc140-mt.dll)

    # TODO later: Fix CMake approach and use config-style package files

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(assimp REQUIRED_VARS ASSIMP_LIBS_DIR)

	add_library(assimp SHARED IMPORTED)
	set_target_properties(assimp PROPERTIES
	    IMPORTED_CONFIGURATIONS "Debug;Release"
	    IMPORTED_LOCATION_RELEASE ${ASSIMP_LIBS}
	    IMPORTED_LOCATION_DEBUG ${ASSIMP_LIBS}
	)

    # Copy over DLL post-build
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:assimp> $<TARGET_FILE_DIR:${PROJECT_NAME}>/
    )
endif()