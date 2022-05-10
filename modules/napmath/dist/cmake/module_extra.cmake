include(${NAP_ROOT}/cmake/dist_shared_crossplatform.cmake)

if(NOT TARGET glm)
    set(GLM_FIND_QUIETLY TRUE)
    find_package(glm REQUIRED)
endif()

# Add GLM
add_include_to_interface_target(mod_napmath ${GLM_INCLUDE_DIRS})

# For backwards compatibility, force identity matrix on construction
target_compile_definitions(${PROJECT_NAME} PUBLIC GLM_FORCE_CTOR_INIT)

if(WIN32)
	add_define_to_interface_target(mod_napmath NOMINMAX)
endif()

# Install thirdparty licenses into lib
install(FILES ${THIRDPARTY_DIR}/glm/copying.txt DESTINATION licenses/glm)
