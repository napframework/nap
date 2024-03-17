list(APPEND CMAKE_MODULE_PATH ${NAP_ROOT}/system_modules/naprender/thirdparty/cmake_find_modules)

find_package(SDL2)
target_include_directories(${PROJECT_NAME} PUBLIC ${SDL2_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARY})
