# Find RTTR
set(RTTR_DIR "${NAP_ROOT}/thirdparty/rttr/cmake")
if(NOT TARGET RTTR::Core)
    find_package(RTTR CONFIG REQUIRED Core)
endif()

# Find Python
# Let find_python find our prepackaged Python in thirdparty
find_python_in_thirdparty()
set(pybind11_DIR "${NAP_ROOT}/thirdparty/pybind11/share/cmake/pybind11")
find_package(pybind11 REQUIRED)

# Find rapidjson
find_package(rapidjson)

if (WIN32)
    find_path(
        NAPRTTI_LIBS_DIR
        NAMES Release/naprtti.lib
        HINTS
            ${CMAKE_CURRENT_LIST_DIR}/../lib/
    )
    set(NAPRTTI_LIBS_DEBUG ${NAPRTTI_LIBS_DIR}/Debug/naprtti.lib)
    set(NAPRTTI_LIBS_RELEASE ${NAPRTTI_LIBS_DIR}/Release/naprtti.lib)
elseif (APPLE)
    find_path(
        NAPRTTI_LIBS_DIR
        NAMES Release/naprtti.dylib
        HINTS
            ${CMAKE_CURRENT_LIST_DIR}/../lib/
    )
    set(NAPRTTI_LIBS_RELEASE ${NAPRTTI_LIBS_DIR}/Release/naprtti.dylib)
    set(NAPRTTI_LIBS_DEBUG ${NAPRTTI_LIBS_DIR}/Debug/naprtti.dylib)
elseif (UNIX)
    find_path(
        NAPRTTI_LIBS_DIR
        NAMES Debug/naprtti.so
        HINTS
            ${CMAKE_CURRENT_LIST_DIR}/../lib/
    )
    set(NAPRTTI_LIBS_RELEASE ${NAPRTTI_LIBS_DIR}/Release/naprtti.so)
    set(NAPRTTI_LIBS_DEBUG ${NAPRTTI_LIBS_DIR}/Debug/naprtti.so)
endif()

# Setup as interface library
add_library(naprtti INTERFACE)
target_link_libraries(naprtti INTERFACE debug ${NAPRTTI_LIBS_DEBUG} RTTR::Core)
target_link_libraries(naprtti INTERFACE optimized ${NAPRTTI_LIBS_RELEASE} RTTR::Core)
set_target_properties(naprtti PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${NAP_ROOT}/include;${pybind11_INCLUDE_DIRS};${RAPIDJSON_INCLUDE_DIRS}")

# Show headers in IDE
file(GLOB rtti_headers ${CMAKE_CURRENT_LIST_DIR}/../include/rtti/*.h)
target_sources(naprtti INTERFACE ${rtti_headers})
source_group(NAP\\RTTI FILES ${rtti_headers})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(naprtti REQUIRED_VARS NAPRTTI_LIBS_DIR)

if (WIN32)
    # Copy over DLL post-build
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${NAPRTTI_LIBS_DIR}/$<CONFIG>/naprtti.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )

    # Copy PDB post-build, if we have them
    if(EXISTS ${NAPRTTI_LIBS_DIR}/Debug/naprtti.pdb)
        add_custom_command(
            TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${NAPRTTI_LIBS_DIR}/$<CONFIG>/naprtti.pdb $<TARGET_FILE_DIR:${PROJECT_NAME}>/            
            )
    endif()

    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:RTTR::Core> $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )

    # Copy Python DLLs post-build on Windows
    win64_copy_python_dlls_postbuild(FALSE)
endif()

# Package naprtti and RTTR into projects for macOS/Linux
if(NOT WIN32)
    install(FILES ${NAPRTTI_LIBS_RELEASE} DESTINATION lib CONFIGURATIONS Release)    
    install(FILES $<TARGET_FILE:RTTR::Core> DESTINATION lib CONFIGURATIONS Release) 

    # Add post-build step to set RTTR RPATH
    if(APPLE)
        add_custom_command(TARGET ${PROJECT_NAME}
                           POST_BUILD
                           COMMAND sh -c \"${CMAKE_INSTALL_NAME_TOOL} -add_rpath ${THIRDPARTY_DIR}/rttr/bin $<TARGET_FILE:${PROJECT_NAME}> 2>/dev/null\;exit 0\"
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

    # Package our Python dylib from thirdparty.  Doing this here instead of in mod_nappython as RTTI (and as a result Core)
    # depend on Python. The Python modules however are only installed if we're using mod_nappython as they're not required 
    # for RTTI/Core.
    file(GLOB PYTHON_DYLIBS ${THIRDPARTY_DIR}/python/lib/lib*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
    install(FILES ${PYTHON_DYLIBS} DESTINATION lib/)

    # Package Python license into packaged project
    install(FILES ${THIRDPARTY_DIR}/python/LICENSE DESTINATION licenses/python/)    
endif()

# Package RTTR license into packaged project
install(FILES ${THIRDPARTY_DIR}/rttr/LICENSE.txt DESTINATION licenses/RTTR/)

# Package pybind license into packaged project
install(FILES ${THIRDPARTY_DIR}/pybind11/LICENSE DESTINATION licenses/pybind11/)

# Package rapidjson license into packaged project
install(FILES ${THIRDPARTY_DIR}/rapidjson/license.txt DESTINATION licenses/rapidjson)
