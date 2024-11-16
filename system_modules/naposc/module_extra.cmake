add_subdirectory(thirdparty/oscpack)
target_link_import_library(${PROJECT_NAME} oscpack)

add_license(oscpack ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/oscpack/source/LICENSE)