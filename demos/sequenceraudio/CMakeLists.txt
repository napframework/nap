cmake_minimum_required(VERSION 3.15.4)

# Exclude for Android
if(ANDROID)
    return()
endif()

project(sequenceraudio)

# add all cpp files to SOURCES
file(GLOB SOURCES src/*.cpp)
file(GLOB HEADERS src/*.h)
file(GLOB SHADERS data/shaders/*.frag data/shaders/*.vert)
file(GLOB DATA data/*)

# Create IDE groups
source_group("headers" FILES ${HEADERS})
source_group("sources" FILES ${SOURCES})
source_group("shaders" FILES ${SHADERS})

# add executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${SHADERS})
set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)")
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Demos)

# Get our modules list from project.json
project_json_to_cmake()

target_link_libraries(${PROJECT_NAME} ${NAP_MODULES})
add_dependencies(${PROJECT_NAME} fbxconverter)

# Package into release, export FBX, other shared source project fixes
nap_source_project_packaging_and_shared_postprocessing(TRUE FALSE "demos" TRUE)