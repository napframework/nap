cmake_minimum_required(VERSION 2.8.12)
project(pynap)


find_path(
        RTTR_DIR
        NAMES rttr-config.cmake
        HINTS
        ${CMAKE_CURRENT_LIST_DIR}/../../../thirdparty/rttr/linux/install/cmake
)


find_package(RTTR CONFIG REQUIRED Core)
find_package(pybind11 REQUIRED)

add_library(pynap_lib src/main.cpp)

pybind11_add_module(${PROJECT_NAME} src/main.cpp)



target_link_libraries(${PROJECT_NAME} PRIVATE napcore python)


# generate python module
set(OUT ${CMAKE_CURRENT_LIST_DIR}/dist)

add_custom_command(
        OUTPUT ${OUT}
        COMMAND python3
        ARGS setup.py bdist
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        DEPENDS ${PROJECT_NAME}
)

add_custom_target(${PROJECT_NAME}_install DEPENDS ${OUT})
