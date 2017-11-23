if (WIN32)
    find_path(
        NAPUTILITY_DEBUG_LIBS_DIR
        NAMES Debug/naputility.lib
        HINTS
            ${CMAKE_CURRENT_LIST_DIR}/../lib/
    )
    set(NAPUTILITY_DEBUG_LIBS_IMPLIB ${NAPUTILITY_DEBUG_LIBS_DIR}/Debug/naputility.lib)
    set(NAPUTILITY_RELEASE_LIBS_IMPLIB ${NAPUTILITY_DEBUG_LIBS_DIR}/Release/naputility.lib)

    add_library(naputility SHARED IMPORTED)
    set_target_properties(naputility PROPERTIES
      IMPORTED_CONFIGURATIONS "Debug;Release;MinSizeRel;RelWithDebInfo"
      IMPORTED_IMPLIB_DEBUG ${NAPUTILITY_DEBUG_LIBS_IMPLIB}
      IMPORTED_IMPLIB_RELEASE ${NAPUTILITY_RELEASE_LIBS_IMPLIB}
      IMPORTED_IMPLIB_MINSIZEREL ${NAPUTILITY_RELEASE_LIBS_IMPLIB}
      IMPORTED_IMPLIB_RELWITHDEBINFO ${NAPUTILITY_RELEASE_LIBS_IMPLIB}
    )

    # TODO later: Fix CMake approach and use config-style package files

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(naputility REQUIRED_VARS NAPUTILITY_DEBUG_LIBS_DIR)
endif()