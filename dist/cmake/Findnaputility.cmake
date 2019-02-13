# Find moodycamel
find_package(moodycamel REQUIRED)

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
elseif (ANDROID)
    find_path(
        NAPUTILITY_LIBS_DIR
        NAMES Release/${ANDROID_ABI}/libnaputility.a
        HINTS ${NAP_ROOT}/lib/
    )
    set(NAPUTILITY_LIBS_RELEASE ${NAPRTTI_LIBS_DIR}/Release/${ANDROID_ABI}/libnaputility.a)
    set(NAPUTILITY_LIBS_DEBUG ${NAPRTTI_LIBS_DIR}/Debug/${ANDROID_ABI}/libnaputility.a)
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

# Setup as interface library
add_library(naputility INTERFACE)
target_link_libraries(naputility INTERFACE optimized ${NAPUTILITY_LIBS_RELEASE})
target_link_libraries(naputility INTERFACE debug ${NAPUTILITY_LIBS_DEBUG})
set_target_properties(naputility PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${NAP_ROOT}/include;${MOODYCAMEL_INCLUDE_DIRS}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(naputility REQUIRED_VARS NAPRTTI_LIBS_DIR)

# Show headers in IDE
file(GLOB utility_headers ${CMAKE_CURRENT_LIST_DIR}/../include/utility/*.h)
target_sources(naputility INTERFACE ${utility_headers})
source_group(NAP\\Utility FILES ${utility_headers})

# Install moodycamel license into packaged project
install(FILES ${THIRDPARTY_DIR}/moodycamel/LICENSE.md DESTINATION licenses/moodycamel)