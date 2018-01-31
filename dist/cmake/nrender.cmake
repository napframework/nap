if (WIN32)
    find_path(
        NRENDER_LIBS_DIR
        NAMES Release/nrender.lib
        HINTS ${CMAKE_CURRENT_LIST_DIR}/../lib/
    )
    set(NRENDER_LIBS_RELEASE ${NRENDER_LIBS_DIR}/Release/nrender.lib)
    set(NRENDER_LIBS_DEBUG ${NRENDER_LIBS_DIR}/Debug/nrender.lib)
elseif (APPLE)
    find_path(
        NRENDER_LIBS_DIR
        NAMES Release/libnrender.a
        HINTS ${CMAKE_CURRENT_LIST_DIR}/../lib/
    )
    set(NRENDER_LIBS_RELEASE ${NRENDER_LIBS_DIR}/Release/libnrender.a)
    set(NRENDER_LIBS_DEBUG ${NRENDER_LIBS_DIR}/Debug/libnrender.a)
elseif (UNIX)
    find_path(
        NRENDER_LIBS_DIR
	NAMES Debug/libnrender.a
        HINTS ${CMAKE_CURRENT_LIST_DIR}/../lib/
    )
    set(NRENDER_LIBS_RELEASE ${NRENDER_LIBS_DIR}/Release/libnrender.a)
    set(NRENDER_LIBS_DEBUG ${NRENDER_LIBS_DIR}/Debug/libnrender.a)
endif()


if (NOT NRENDER_LIBS_DIR)
    message(FATAL_ERROR "Couldn't find NRender")
endif()

add_library(nrender INTERFACE)
# TODO re-enable once dependency issues are solved
# target_link_libraries(nrender INTERFACE optimized ${NRENDER_LIBS_RELEASE})
# target_link_libraries(nrender INTERFACE debug ${NRENDER_LIBS_DEBUG})
file(GLOB nrender_headers ${CMAKE_CURRENT_LIST_DIR}/../include/nrender/*.h)
target_sources(nrender INTERFACE ${nrender_headers})
source_group(NAP\\NRender FILES ${nrender_headers})