set(APP_CUSTOM_IDE_FOLDER Test)

# Let find_python find our prepackaged Python in thirdparty
configure_python()
set(pybind11_DIR "${NAP_ROOT}/system_modules/nappython/thirdparty/pybind11/install/share/cmake/pybind11")
find_package(pybind11)

target_include_directories(${PROJECT_NAME} PUBLIC ${pybind11_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PUBLIC ${PYTHON_LIB_DIR})
