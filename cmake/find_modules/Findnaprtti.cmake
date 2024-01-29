# Find RTTR
find_rttr()

# Find rapidjson
find_package(rapidjson)

# Let find_python find our prepackaged Python in thirdparty,
# Note that this operation is allowed to fail because, by default, Python support is disabled.
configure_python()
set(pybind11_DIR "${NAP_ROOT}/system_modules/nappython/thirdparty/pybind11/install/share/cmake/pybind11")
find_package(pybind11 QUIET)

if(WIN32)
    find_path(
        NAPRTTI_LIBS_DIR
        NAMES Release-${ARCH}/naprtti.lib
        HINTS ${NAP_ROOT}/lib/
    )
    set(NAPRTTI_LIBS_DEBUG ${NAPRTTI_LIBS_DIR}/Debug-${ARCH}/naprtti.lib)
    set(NAPRTTI_LIBS_RELEASE ${NAPRTTI_LIBS_DIR}/Release-${ARCH}/naprtti.lib)
elseif(UNIX)
    find_path(
        NAPRTTI_LIBS_DIR
        NAMES Debug-${ARCH}/naprtti${CMAKE_SHARED_LIBRARY_SUFFIX}
        HINTS ${NAP_ROOT}/lib/
    )
    set(NAPRTTI_LIBS_RELEASE ${NAPRTTI_LIBS_DIR}/Release-${ARCH}/naprtti${CMAKE_SHARED_LIBRARY_SUFFIX})
    set(NAPRTTI_LIBS_DEBUG ${NAPRTTI_LIBS_DIR}/Debug-${ARCH}/naprtti${CMAKE_SHARED_LIBRARY_SUFFIX})
endif()

# Setup as interface library
add_library(naprtti INTERFACE)
target_link_libraries(naprtti INTERFACE debug ${NAPRTTI_LIBS_DEBUG} RTTR::Core)
target_link_libraries(naprtti INTERFACE optimized ${NAPRTTI_LIBS_RELEASE} RTTR::Core)
target_link_directories(naprtti INTERFACE ${PYTHON_LIB_DIR})
set_target_properties(naprtti PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${NAP_ROOT}/include;${pybind11_INCLUDE_DIRS};${RAPIDJSON_INCLUDE_DIRS}")

# Show headers in IDE
file(GLOB rtti_headers ${CMAKE_CURRENT_LIST_DIR}/../include/rtti/*.h)
target_sources(naprtti INTERFACE ${rtti_headers})
source_group(NAP\\RTTI FILES ${rtti_headers})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(naprtti REQUIRED_VARS NAPRTTI_LIBS_DIR)

# Package naprtti and RTTR into apps for Windows
if(WIN32)
    # Copy over DLL post-build
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${NAPRTTI_LIBS_DIR}/$<CONFIG>-${ARCH}/naprtti.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )

    # Copy PDB post-build, if we have them
    if(EXISTS ${NAPRTTI_LIBS_DIR}/Debug/naprtti.pdb)
        add_custom_command(
            TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${NAPRTTI_LIBS_DIR}/$<CONFIG>-${ARCH}/naprtti.pdb $<TARGET_FILE_DIR:${PROJECT_NAME}>/
            )
    endif()

    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:RTTR::Core> $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )

    # Copy Python DLLs post-build on Windows
    if(pybind11_FOUND)
        win64_copy_python_dlls_postbuild(FALSE)
    endif()
endif()

# Package naprtti and RTTR into apps for macOS/Linux
if(NOT WIN32)
    install(FILES ${NAPRTTI_LIBS_RELEASE} DESTINATION lib CONFIGURATIONS Release)
    install(FILES $<TARGET_FILE:RTTR::Core> DESTINATION lib CONFIGURATIONS Release)

    # Add post-build step to set RTTR RPATH
    if(APPLE)
        add_custom_command(TARGET ${PROJECT_NAME}
                           POST_BUILD
                           COMMAND sh -c \"${CMAKE_INSTALL_NAME_TOOL} -add_rpath ${THIRDPARTY_DIR}/rttr/macos/${ARCH}/bin $<TARGET_FILE:${PROJECT_NAME}> 2>/dev/null\;exit 0\"
                           COMMAND codesign -f -s -  $<TARGET_FILE:${PROJECT_NAME}>
                           )
    endif()

    # On Linux use lib directory for RPATH
    if(UNIX AND NOT APPLE)
        install(CODE "message(\"Setting RPATH on ${CMAKE_INSTALL_PREFIX}/lib/naprtti.so\")
                      execute_process(COMMAND patchelf
                                              --set-rpath
                                              $ORIGIN/.
                                              ${CMAKE_INSTALL_PREFIX}/lib/naprtti.so
                                      RESULT_VARIABLE EXIT_CODE)
                      if(NOT \${EXIT_CODE} EQUAL 0)
                          message(FATAL_ERROR \"Failed to fetch RPATH on naprtti.so using patchelf\")
                      endif()
                      ")
    endif()

    # Package our Python dylib from thirdparty. Doing this here instead of in nappython as RTTI (and as a result Core)
    # depends on Python. The Python modules however are only installed if we're using nappython as they're not required
    # for RTTI/Core.
    if(pybind11_FOUND)
        file(GLOB PYTHON_DYLIBS ${PYTHON_LIB_DIR}/lib*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
        install(FILES ${PYTHON_DYLIBS} DESTINATION lib/)
    endif()
endif()

# Package RTTR license into packaged app
install(FILES ${THIRDPARTY_DIR}/rttr/source/LICENSE.txt DESTINATION licenses/RTTR/)

# Package Python & pybind11 license into packaged app
if(pybind11_FOUND)
    if(UNIX)
        install(FILES ${PYTHON_LIB_DIR}/python3.6/LICENSE.txt DESTINATION licenses/python/)
    else()
        install(FILES ${PYTHON_PREFIX}/../LICENSE.txt DESTINATION licenses/python/)
    endif()
    install(FILES ${NAP_ROOT}/system_modules/nappython/thirdparty/pybind11/LICENSE DESTINATION licenses/pybind11/)
endif()

# Package RapidJSON license into packaged app
install(FILES ${THIRDPARTY_DIR}/rapidjson/license.txt DESTINATION licenses/RapidJSON/)
