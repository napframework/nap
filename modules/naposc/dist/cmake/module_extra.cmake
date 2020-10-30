include(${NAP_ROOT}/cmake/dist_shared_crossplatform.cmake)

if(NOT TARGET oscpack)
    find_package(oscpack REQUIRED)
endif()
set(MODULE_NAME_EXTRA_LIBS oscpack)

add_include_to_interface_target(mod_naposc ${OSCPACK_INCLUDE_DIRS})

# Install oscpack licenses into packaged project
install(FILES ${THIRDPARTY_DIR}/oscpack/LICENSE DESTINATION licenses/oscpack)

# Install oscpack shared lib into packaged project for Unix
if(APPLE)
    install(FILES $<TARGET_FILE:oscpack> DESTINATION lib)    
elseif(UNIX)
    file(GLOB OSCPACK_DYLIBS ${THIRDPARTY_DIR}/oscpack/lib/liboscpack*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
    install(FILES ${OSCPACK_DYLIBS} DESTINATION lib)
endif()
