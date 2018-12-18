project(mod_napmath)

file(GLOB sources src/*.cpp src/*.h)

# Get our NAP modules dependencies from module.json
module_json_to_cmake()

find_package(glm REQUIRED)

add_library(${PROJECT_NAME} SHARED ${sources})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)

target_include_directories(${PROJECT_NAME} PUBLIC src ${GLM_INCLUDE_DIRS})

target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY PRIVATE _USE_MATH_DEFINES)
target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} napcore)

# Deploy module.json as MODULENAME.json alongside module post-build
copy_module_json_to_bin()

# Package into platform release
package_module()

if (WIN32)
    # Install for fbxconverter
    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION tools/platform
    		CONFIGURATIONS Release)
endif()
