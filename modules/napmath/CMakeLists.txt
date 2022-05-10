cmake_minimum_required(VERSION 3.18.4)
project(mod_napmath)

file(GLOB sources src/*.cpp src/*.h)

# Get our NAP modules dependencies from module.json
module_json_to_cmake()

set(GLM_FIND_QUIETLY TRUE)
find_package(glm REQUIRED)

add_library(${PROJECT_NAME} SHARED ${sources})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)
# Remove lib prefix on Unix libraries
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

# Include GLM
target_include_directories(${PROJECT_NAME} PUBLIC src ${GLM_INCLUDE_DIRS})

# For backwards compatibility, force identity matrix on construction
target_compile_definitions(${PROJECT_NAME} PUBLIC GLM_FORCE_CTOR_INIT)

# setup definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY _USE_MATH_DEFINES MODULE_NAME=${PROJECT_NAME})
if(WIN32)
	target_compile_definitions(${PROJECT_NAME} PUBLIC NOMINMAX)
endif()

# libraries to link to
target_link_libraries(${PROJECT_NAME} ${DEPENDENT_NAP_MODULES} napcore)

# Deploy module.json as MODULENAME.json alongside module post-build
copy_module_json_to_bin()

# Package into platform release
package_module()

# Package GLM
install(FILES ${GLM_DIR}/copying.txt DESTINATION thirdparty/glm)
install(DIRECTORY ${GLM_DIR}/glm/ DESTINATION thirdparty/glm)

if (WIN32)
    # Install for fbxconverter
    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION tools/platform
    		CONFIGURATIONS Release)
endif()
