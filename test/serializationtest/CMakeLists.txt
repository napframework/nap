project(serializationtest)

file(GLOB sources src/*.cpp src/*.h)
include_directories(src)

find_package(rapidjson REQUIRED)
add_definitions(-DRAPIDJSON_HAS_STDSTRING=1)
include_directories(${RAPIDJSON_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} ${sources})
set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)")
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Test)
target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME})

# Get our modules list from project.json
project_json_to_cmake()

target_link_libraries(${PROJECT_NAME} napcore ${NAP_MODULES} nrender)

# Package into release, export FBX, other shared source project fixes
nap_source_project_packaging_and_shared_postprocessing(FALSE FALSE "unused" FALSE)