cmake_minimum_required(VERSION 2.8.12)
project(nap)


# Prerequisites
find_package(pybind11 REQUIRED)


# Library
set(LIBS napcore mod_naprender mod_napinput mod_napscene)
file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)
pybind11_add_module(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBS})
add_definitions(-DNAP_DISABLE_EMBEDDED_PYTHON)


# Install
install(FILES tests/test.py DESTINATION ${LIB_DIR})
add_custom_target(${PROJECT_NAME}_install
        $(MAKE) install
        DEPENDS ${PROJECT_NAME}
        COMMENT "Installing ${PROJECT_NAME}")


# Test
set(OUT out)
add_custom_target(${PROJECT_NAME}_test DEPENDS ${OUT})
add_custom_command(
        OUTPUT ${OUT}
        COMMAND python3
        ARGS test.py
        WORKING_DIRECTORY ${LIB_DIR}
        DEPENDS ${PROJECT_NAME}_install
        COMMENT "Running tests..."
)
