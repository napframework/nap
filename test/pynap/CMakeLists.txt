cmake_minimum_required(VERSION 2.8.12)
project(nap)


# Prerequisites
set(pybind11_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../thirdparty/pybind11/install/share/cmake/pybind11")
find_package(pybind11 REQUIRED)

set(PYNAP_DIR ${CMAKE_CURRENT_LIST_DIR}/../../napkin/lib)

set(PYNAP_DEPS napcore mod_naprender mod_napinput mod_napscene)

# Library
set(LIBS ${PYNAP_DEPS})
file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)
pybind11_add_module(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} PUBLIC ${LIBS})
add_definitions(-DNAP_DISABLE_EMBEDDED_PYTHON)


# Install
install(FILES tests/test.py DESTINATION ${PYNAP_DIR})
install(TARGETS ${PROJECT_NAME} DESTINATION ${PYNAP_DIR})
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
        WORKING_DIRECTORY ${PYNAP_DIR}
        DEPENDS ${PROJECT_NAME}_install
        COMMENT "Running tests..."
)
