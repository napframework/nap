include(${NAP_ROOT}/cmake/dist_shared_crossplatform.cmake)

if(NOT TARGET glm)
    find_package(glm REQUIRED)
endif()

add_include_to_interface_target(mod_napsequencegui ${GLM_INCLUDE_DIRS})