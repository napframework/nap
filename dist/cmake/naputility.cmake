if (WIN32)
    find_path(
        NAPUTILITY_LIBS_DIR
        NAMES Release/naputility.lib
        HINTS ${CMAKE_CURRENT_LIST_DIR}/../lib/
    )
    set(NAPUTILITY_LIBS_RELEASE ${NAPUTILITY_LIBS_DIR}/Release/naputility.lib)
    set(NAPUTILITY_LIBS_DEBUG ${NAPUTILITY_LIBS_DIR}/Debug/naputility.lib)
elseif (APPLE)
    find_path(
        NAPUTILITY_LIBS_DIR
        NAMES Release/libnaputility.a
        HINTS ${CMAKE_CURRENT_LIST_DIR}/../lib/
    )
    set(NAPUTILITY_LIBS_RELEASE ${NAPUTILITY_LIBS_DIR}/Release/libnaputility.a)
    set(NAPUTILITY_LIBS_DEBUG ${NAPUTILITY_LIBS_DIR}/Debug/libnaputility.a)
elseif (UNIX)
    find_path(
        NAPUTILITY_LIBS_DIR
	NAMES Debug/libnaputility.a
        HINTS ${CMAKE_CURRENT_LIST_DIR}/../lib/
    )
    set(NAPUTILITY_LIBS_RELEASE ${NAPUTILITY_LIBS_DIR}/Release/libnaputility.a)
    set(NAPUTILITY_LIBS_DEBUG ${NAPUTILITY_LIBS_DIR}/Debug/libnaputility.a)
endif()


if (NOT NAPUTILITY_LIBS_DIR)
    message(FATAL_ERROR "Couldn't find NAP utility")
endif()

add_library(naputility INTERFACE)
target_link_libraries(naputility INTERFACE optimized ${NAPUTILITY_LIBS_RELEASE})
target_link_libraries(naputility INTERFACE debug ${NAPUTILITY_LIBS_DEBUG})
file(GLOB utility_headers ${CMAKE_CURRENT_LIST_DIR}/../include/utility/*.h)
target_sources(naputility INTERFACE ${utility_headers})
source_group(NAP\\Utility FILES ${utility_headers})