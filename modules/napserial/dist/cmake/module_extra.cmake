include(${NAP_ROOT}/cmake/dist_shared_crossplatform.cmake)

if(NOT TARGET serial)
    find_package(serial REQUIRED)
endif()
set(MODULE_NAME_EXTRA_LIBS serial)

add_include_to_interface_target(mod_serial ${SERIAL_INCLUDE_DIRS})

# Install oscpack shared lib into packaged project for Unix
if(APPLE)
    #install(FILES $<TARGET_FILE:oscpack> DESTINATION lib)    
elseif(UNIX)
    #file(GLOB OSCPACK_DYLIBS ${THIRDPARTY_DIR}/oscpack/lib/liboscpack*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
    #install(FILES ${OSCPACK_DYLIBS} DESTINATION lib)
endif()