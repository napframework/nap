project(pyinterpreter)

find_package(PythonLibs REQUIRED)

file(GLOB sources *.cpp)
include_directories(src ${PYTHON_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} ${sources})

target_link_libraries(${PROJECT_NAME} napcore pthread ${PYTHON_LIBRARIES})