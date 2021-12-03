# Exclude for Android
if(ANDROID)
    return()
endif()

project(fbxconverter)

file(GLOB sources src/*.cpp src/*.h)
include_directories(src)

# Add TCLAP
set(TCLAP_FIND_QUIETLY TRUE)
find_package(tclap REQUIRED)
include_directories(${TCLAP_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} ${sources})
set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)")
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Tools)
target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME})
set(DEPENDENT_NAP_MODULES mod_naprender mod_napmath mod_napscene)
target_link_libraries(${PROJECT_NAME} napcore ${DEPENDENT_NAP_MODULES})

# Add the runtime paths for RTTR on macOS
if(APPLE)
    add_macos_rttr_rpath()
endif()

# ======================= UNIT TESTS
enable_testing()

# ensure failure without arguments
add_test(NAME NoArguments COMMAND ${PROJECT_NAME})
set_tests_properties(NoArguments PROPERTIES WILL_FAIL true)

# convert an fbx
add_test(NAME ConvertFile COMMAND ${PROJECT_NAME} -o
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/res-test/vinyl_cover_converted.mesh
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/res-test/vinyl_cover.fbx)


# dummy command. When depended on, the target wil always be run
add_custom_command(OUTPUT always_rebuild COMMAND cmake -E echo)

# copy test data
add_custom_command(TARGET ${PROJECT_NAME} 
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different
                           ${CMAKE_CURRENT_LIST_DIR}/res/vinyl_cover.fbx
                           $<TARGET_FILE_DIR:${PROJECT_NAME}>/res-test/vinyl_cover.fbx
                   DEPENDS always_rebuild
                   )

# auto run test
#add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#        COMMAND ctest -C $<CONFIGURATION> --output-on-failure
#        DEPENDS always_rebuild)

# ==================================

# Package into NAP release
set(FBXCONVERTER_PACKAGED_BUILD_TYPE Release)
set(FBXCONVERTER_INSTALL_LOCATION tools/platform)

install(TARGETS ${PROJECT_NAME} 
        DESTINATION ${FBXCONVERTER_INSTALL_LOCATION}
        CONFIGURATIONS ${FBXCONVERTER_PACKAGED_BUILD_TYPE})

if(UNIX)
    # Extra RPATH building for Linux and macOS
    set(PATH_TO_NAP_ROOT "../..")
    set(EXTRA_RPATH ${PATH_TO_NAP_ROOT}/thirdparty/assimp/lib)
    list(APPEND EXTRA_RPATH ${PATH_TO_NAP_ROOT}/thirdparty/SDL2/lib)
    list(APPEND EXTRA_RPATH ${PATH_TO_NAP_ROOT}/thirdparty/FreeImage/lib)
    list(APPEND EXTRA_RPATH ${PATH_TO_NAP_ROOT}/thirdparty/freetype/lib)
    list(APPEND EXTRA_RPATH ${PATH_TO_NAP_ROOT}/thirdparty/vulkansdk/lib)
endif()

# Add extra modules for RPATH building on Linux & macOS, likely related to NAP-225
list(APPEND DEPENDENT_NAP_MODULES mod_napfont mod_napinput mod_napcolor)
if (WIN32)   
    file(GLOB PYTHON_DLLS ${THIRDPARTY_DIR}/python/msvc/x86_64/*.dll)
    # TODO Update to find the libraries then install locating them via TARGET_FILE
    install(FILES ${THIRDPARTY_DIR}/assimp/msvc/x86_64/bin/assimp-vc140-mt.dll
                  ${THIRDPARTY_DIR}/FreeImage/msvc/x86_64/Dist/x64/FreeImage.dll
                  ${THIRDPARTY_DIR}/rttr/msvc/x86_64/bin/rttr_core.dll
                  ${THIRDPARTY_DIR}/SDL2/msvc/x86_64/lib/SDL2.dll
                  ${THIRDPARTY_DIR}/freetype/msvc/x86_64/lib/Release/freetype.dll
                  ${PYTHON_DLLS}
            DESTINATION ${FBXCONVERTER_INSTALL_LOCATION})

    if(PACKAGE_PDBS)
        install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> 
                DESTINATION ${FBXCONVERTER_INSTALL_LOCATION}
                CONFIGURATIONS ${FBXCONVERTER_PACKAGED_BUILD_TYPE}
                )
    endif()            
elseif(APPLE)
    list(APPEND EXTRA_RPATH ${PATH_TO_NAP_ROOT}/lib/${FBXCONVERTER_PACKAGED_BUILD_TYPE})

    set_single_config_installed_rpath_on_macos_object_for_dependent_modules(${FBXCONVERTER_PACKAGED_BUILD_TYPE} 
                                                                            "${DEPENDENT_NAP_MODULES}" 
                                                                            ${CMAKE_INSTALL_PREFIX}/tools/platform/fbxconverter
                                                                            "../.."
                                                                            "${EXTRA_RPATH}")
elseif(UNIX)
    set_installed_rpath_on_linux_object_for_dependent_modules("${DEPENDENT_NAP_MODULES}" ${PROJECT_NAME} "../.." "${EXTRA_RPATH}")
endif()

# Add tclap license into release
install(FILES ${THIRDPARTY_DIR}/tclap-1.2.1/README
              ${THIRDPARTY_DIR}/tclap-1.2.1/COPYING
        DESTINATION thirdparty/tclap)
install(DIRECTORY ${TCLAP_INCLUDE_DIRS}/tclap/ DESTINATION thirdparty/tclap/include/tclap)
