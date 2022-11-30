# Find moodycamel
find_package(moodycamel REQUIRED)

find_path(
    NAPUTILITY_LIBS_DIR
    NAMES Debug-${ARCH}/naputility${CMAKE_STATIC_LIBRARY_SUFFIX}
    HINTS ${CMAKE_CURRENT_LIST_DIR}/../lib/
)
set(NAPUTILITY_LIBS_RELEASE ${NAPUTILITY_LIBS_DIR}/Release-${ARCH}/naputility${CMAKE_STATIC_LIBRARY_SUFFIX})
set(NAPUTILITY_LIBS_DEBUG ${NAPUTILITY_LIBS_DIR}/Debug-${ARCH}/naputility${CMAKE_STATIC_LIBRARY_SUFFIX})

if(NOT NAPUTILITY_LIBS_DIR)
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
