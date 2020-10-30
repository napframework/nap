include(${NAP_ROOT}/cmake/dist_shared_crossplatform.cmake)

if(NOT TARGET freetype)
    find_package(freetype REQUIRED)
endif()

set(MODULE_NAME_EXTRA_LIBS freetype)
add_include_to_interface_target(mod_napfont ${FREETYPE_INCLUDE_DIRS})

if(WIN32)
    copy_freetype_dll()
elseif(APPLE)
    file(GLOB RTFREETYPE_LIBS ${THIRDPARTY_DIR}/freetype/lib/libfreetype*${CMAKE_SHARED_LIBRARY_SUFFIX})
      install(FILES ${RTFREETYPE_LIBS} DESTINATION lib)
elseif(UNIX)
    file(GLOB RTFREETYPE_LIBS ${THIRDPARTY_DIR}/freetype/lib/libfree*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
      install(FILES ${RTFREETYPE_LIBS} DESTINATION lib)
endif()
