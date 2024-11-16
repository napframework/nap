include(${NAP_ROOT}/cmake/nap_utilities.cmake)

add_subdirectory(thirdparty/freetype)
target_link_import_library(${PROJECT_NAME} freetype)

add_license(freetype ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/freetype/source/README)
