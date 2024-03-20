list(APPEND CMAKE_MODULE_PATH ${NAP_ROOT}/system_modules/naprender/thirdparty/cmake_find_modules)

include(${NAP_ROOT}/cmake/module_utilities.cmake)
target_link_import_library(${PROJECT_NAME} SDL2)