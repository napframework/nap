project(mod_napaudioadvanced)

file(GLOB_RECURSE sources src/*.cpp src/*.h)

source_group("core" src/audio/core/*.*)
source_group("node" src/audio/node/*.*)
source_group("object" src/audio/object/*.*)
source_group("component" src/audio/component/*.*)
source_group("resource" src/audio/resource/*.*)
source_group("service" src/audio/service/*.*)
source_group("utility" src/audio/utility/*.*)

# Get our NAP modules dependencies from module.json
module_json_to_cmake()

if (APPLE)
    set(LIBRARIES napcore)
else()
    set(LIBRARIES napcore)
endif()

set(INCLUDES src)

add_library(${PROJECT_NAME} SHARED ${sources})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)

target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDES})
target_link_libraries(${PROJECT_NAME} ${LIBRARIES} ${DEPENDENT_NAP_MODULES})

target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)
target_compile_definitions(${PROJECT_NAME} PRIVATE _USE_MATH_DEFINES)

# Deploy module.json as MODULENAME.json alongside module post-build
copy_module_json_to_bin()

# Package into platform release
package_module()
