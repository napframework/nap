find_package(glm REQUIRED)

# Include GLM
target_include_directories(${PROJECT_NAME} PUBLIC ${GLM_INCLUDE_DIRS})

# For backwards compatibility, force identity matrix on construction
target_compile_definitions(${PROJECT_NAME} PUBLIC GLM_FORCE_CTOR_INIT)

# Setup definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE _USE_MATH_DEFINES MODULE_NAME=${PROJECT_NAME})
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PUBLIC NOMINMAX)
endif()

# Copy glm license
add_license(glm ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glm/copying.txt)
