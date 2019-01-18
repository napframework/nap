set(MODNAPSCENE_LIBS_RELEASE ${NAP_ROOT}/modules/mod_napscene/lib/Release/${ANDROID_ABI}/libmod_napscene.so)
set(MODNAPSCENE_LIBS_DEBUG ${NAP_ROOT}/modules/mod_napscene/lib/Debug/${ANDROID_ABI}/libmod_napscene.so)
add_library(mod_napscene INTERFACE)
target_link_libraries(mod_napscene INTERFACE optimized ${MODNAPSCENE_LIBS_RELEASE})
target_link_libraries(mod_napscene INTERFACE debug ${MODNAPSCENE_LIBS_DEBUG})

if(NOT TARGET glm)
    find_package(glm REQUIRED)
endif()

set_target_properties(mod_napscene PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${NAP_ROOT}/modules/mod_napscene/include;${GLM_INCLUDE_DIRS}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(mod_napscene REQUIRED_VARS MODNAPSCENE_LIBS_RELEASE MODNAPSCENE_LIBS_DEBUG)