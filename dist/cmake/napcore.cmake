if (WIN32)
    find_path(
        NAPCORE_LIBS_DIR
        NAMES Release/napcore.lib
        HINTS ${CMAKE_CURRENT_LIST_DIR}/../lib/
    )
    set(NAPCORE_LIBS_RELEASE_DLL ${NAPCORE_LIBS_DIR}/Release/napcore.dll)
    set(NAPCORE_LIBS_DEBUG_DLL ${NAPCORE_LIBS_DIR}/Debug/napcore.dll)
    set(NAPCORE_LIBS_IMPLIB_DEBUG ${NAPCORE_LIBS_DIR}/Debug/napcore.lib)
    set(NAPCORE_LIBS_IMPLIB_RELEASE ${NAPCORE_LIBS_DIR}/Release/napcore.lib)
elseif (APPLE)
    find_path(
        NAPCORE_LIBS_DIR
        NAMES Release/libnapcore.dylib
        HINTS ${CMAKE_CURRENT_LIST_DIR}/../lib/
    )
    set(NAPCORE_LIBS_RELEASE_DLL ${NAPCORE_LIBS_DIR}/Release/libnapcore.dylib)
    set(NAPCORE_LIBS_DEBUG_DLL ${NAPCORE_LIBS_DIR}/Debug/libnapcore.dylib)
elseif (UNIX)
    find_path(
        NAPCORE_LIBS_DIR
	NAMES Debug/libnapcore.so
        HINTS ${CMAKE_CURRENT_LIST_DIR}/../lib/
    )
    set(NAPCORE_LIBS_RELEASE_DLL ${NAPCORE_LIBS_DIR}/Release/libnapcore.so)
    set(NAPCORE_LIBS_DEBUG_DLL ${NAPCORE_LIBS_DIR}/Debug/libnapcore.so)
endif()

if (NOT NAPCORE_LIBS_DIR)
    message(FATAL_ERROR "Couldn't find NAP core")
endif()

add_library(napcore INTERFACE)
target_link_libraries(napcore INTERFACE debug ${NAPCORE_LIBS_DEBUG_DLL})
target_link_libraries(napcore INTERFACE optimized ${NAPCORE_LIBS_RELEASE_DLL})
file(GLOB core_headers ${CMAKE_CURRENT_LIST_DIR}/../include/nap/*.h)
target_sources(napcore INTERFACE ${core_headers})
source_group(NAP\\Core FILES ${core_headers})

file(GLOB utility_headers ${CMAKE_CURRENT_LIST_DIR}/../include/utility/*.h)
target_sources(napcore INTERFACE ${utility_headers})
source_group(NAP\\Utility FILES ${utility_headers})


if (WIN32)
    set_target_properties(napcore PROPERTIES
        IMPORTED_IMPLIB_RELEASE ${NAPCORE_LIBS_IMPLIB_RELEASE}
        IMPORTED_IMPLIB_DEBUG ${NAPCORE_LIBS_IMPLIB_DEBUG}
        IMPORTED_IMPLIB_MINSIZEREL ${NAPCORE_LIBS_IMPLIB_RELEASE}
        IMPORTED_IMPLIB_RELWITHDEBINFO ${NAPCORE_LIBS_IMPLIB_RELEASE}
    )

    # Find our naputility import lib
    find_package(naputility REQUIRED)
    target_link_libraries(${PROJECT_NAME} naputility)

    # Copy over DLL post-build
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:napcore> $<TARGET_FILE_DIR:${PROJECT_NAME}>/
    )
endif()