include(${NAP_ROOT}/cmake/module_utilities.cmake)

add_subdirectory(thirdparty/freetype)
target_link_import_library(${PROJECT_NAME} freetype)