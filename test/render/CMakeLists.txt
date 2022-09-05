cmake_minimum_required(VERSION 3.18.4)
project(render)

#add all cpp files to SOURCES
file(GLOB SOURCES src/*.cpp)
file(GLOB HEADERS src/*.h)
file(GLOB SHADERS data/shaders/*.frag data/shaders/*.vert)
file(GLOB DATA data/*)

# Create IDE groups
source_group("headers" FILES ${HEADERS})
source_group("sources" FILES ${SOURCES})
source_group("shaders" FILES ${SHADERS})

# Let find_python find our prepackaged Python in thirdparty
find_python_in_thirdparty()
set(pybind11_DIR "${THIRDPARTY_DIR}/pybind11/install/share/cmake/pybind11")
find_package(pybind11)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${SHADERS})
set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)")
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Test)
add_dependencies(${PROJECT_NAME} fbxconverter mod_nappython)

# Get our modules list from project.json
project_json_to_cmake()

target_include_directories(${PROJECT_NAME} PUBLIC ${pybind11_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} napcore ${NAP_MODULES})
target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME})

# Package into release, export FBX, other shared source project fixes
# Note: This is for now packaged along with NAIVI apps into projects so that we have a project to test using mod_nappython
nap_source_project_packaging_and_shared_postprocessing(TRUE TRUE "projects" TRUE)