# Find RTTR
set(RTTR_DIR "${NAP_ROOT}/thirdparty/rttr/cmake")
find_package(RTTR CONFIG REQUIRED Core)

# Find Python
# Let find_python find our prepackaged Python in thirdparty
find_python_in_thirdparty()
set(pybind11_DIR "${NAP_ROOT}/thirdparty/pybind11/share/cmake/pybind11")
find_package(pybind11 REQUIRED)
target_include_directories(${PROJECT_NAME} PUBLIC ${pybind11_INCLUDE_DIRS})

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
        NAMES Release/libnaprtti.dylib
        HINTS
            ${CMAKE_CURRENT_LIST_DIR}/../lib/
    )
    set(NAPRTTI_LIBS_RELEASE ${NAPRTTI_LIBS_DIR}/Release/libnaprtti.dylib)
    set(NAPRTTI_LIBS_DEBUG ${NAPRTTI_LIBS_DIR}/Debug/libnaprtti.dylib)
elseif (UNIX)
    find_path(
        NAPRTTI_LIBS_DIR
	NAMES Debug/libnaprtti.so
        HINTS
            ${CMAKE_CURRENT_LIST_DIR}/../lib/
    )
    set(NAPRTTI_LIBS_RELEASE ${NAPRTTI_LIBS_DIR}/Release/libnaprtti.so)
    set(NAPRTTI_LIBS_DEBUG ${NAPRTTI_LIBS_DIR}/Debug/libnaprtti.so)
endif()

if (NOT NAPRTTI_LIBS_DIR)
    message(FATAL_ERROR "Couldn't find NAP RTTI")
endif()

# Setup as interface library
add_library(naprtti INTERFACE)
target_link_libraries(naprtti INTERFACE debug ${NAPRTTI_LIBS_DEBUG})
target_link_libraries(naprtti INTERFACE optimized ${NAPRTTI_LIBS_RELEASE})

# Show headers in IDE
file(GLOB rtti_headers ${CMAKE_CURRENT_LIST_DIR}/../include/rtti/*.h)
target_sources(naprtti INTERFACE ${rtti_headers})
source_group(NAP\\RTTI FILES ${rtti_headers})

if (WIN32)
    # Copy over DLL post-build
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${NAPRTTI_LIBS_DIR}/$<CONFIG>/naprtti.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )

    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:RTTR::Core> $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )

    # Copy Python DLLs post-build on Windows
    win64_copy_python_dlls_postbuild()
endif()

# Install naprtti and RTTR into projects for macOS/Linux
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
        install(CODE "message(\"Setting RPATH on ${CMAKE_INSTALL_PREFIX}/lib/libnaprtti.so\")
                      execute_process(COMMAND patchelf 
                                              --set-rpath 
                                              $ORIGIN/.
                                              ${CMAKE_INSTALL_PREFIX}/lib/libnaprtti.so
                                              )
                      ")
    endif()   

    # Install our Python dylib from thirdparty.  Doing this here instead of in mod_nappython as RTTI (and as a result Core)
    # depend on Python. The Python modules however are only installed if we're using mod_nappython as they're not required 
    # for RTTI/Core.
    file(GLOB PYTHON_DYLIBS ${THIRDPARTY_DIR}/python/lib/lib*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
    install(FILES ${PYTHON_DYLIBS} DESTINATION lib/)

    # Install Python license into packaged project
    install(FILES ${THIRDPARTY_DIR}/python/LICENSE DESTINATION licenses/python/)    
endif()

# Install RTTR license into packaged project
install(FILES ${THIRDPARTY_DIR}/rttr/LICENSE.txt DESTINATION licenses/RTTR/)

# Install pybind license into packaged project
install(FILES ${THIRDPARTY_DIR}/pybind11/LICENSE DESTINATION licenses/pybind11/)