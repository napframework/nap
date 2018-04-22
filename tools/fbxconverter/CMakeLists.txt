project(fbxconverter)

file(GLOB sources src/*.cpp src/*.h)
include_directories(src)

# Add TCLAP
find_package(tclap REQUIRED)
include_directories(${TCLAP_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} ${sources})
set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)")
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Tools)
target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME})

if (MSVC)
	# Link against setargv.obj so that the shell does wildcard expansion for us
	# See https://msdn.microsoft.com/en-us/library/8bch7bkk.aspx
	set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "setargv.obj")
endif()

set(DEPENDENT_NAP_MODULES mod_naprender mod_napmath mod_napscene)
target_link_libraries(${PROJECT_NAME} napcore nrender ${DEPENDENT_NAP_MODULES})

if(APPLE)
    # Add the runtime path for RTTR on macOS
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

if (WIN32)   
    file(GLOB PYTHON_DLLS ${THIRDPARTY_DIR}/python/msvc/python-embed-amd64/*.dll)
    # TODO Update to find the libraries then install locating them via TARGET_FILE
    install(FILES ${THIRDPARTY_DIR}/assimp/msvc64/install/bin/assimp-vc140-mt.dll
                  ${THIRDPARTY_DIR}/FreeImage/msvc/Dist/x64/FreeImage.dll
                  ${THIRDPARTY_DIR}/glew/msvc/bin/Release/x64/glew32.dll
                  ${THIRDPARTY_DIR}/rttr/msvc64/install/bin/rttr_core.dll
                  ${THIRDPARTY_DIR}/SDL2/msvc/lib/x64/SDL2.dll
                  ${PYTHON_DLLS}
            DESTINATION ${FBXCONVERTER_INSTALL_LOCATION})
elseif(APPLE)
    set(PATH_TO_NAP_ROOT "../..")
    set(EXTRA_RPATH ${PATH_TO_NAP_ROOT}/thirdparty/assimp/lib)
    list(APPEND EXTRA_RPATH ${PATH_TO_NAP_ROOT}/thirdparty/glew/lib)
    list(APPEND EXTRA_RPATH ${PATH_TO_NAP_ROOT}/thirdparty/SDL2/lib)
    list(APPEND EXTRA_RPATH ${PATH_TO_NAP_ROOT}/thirdparty/FreeImage/lib)
    list(APPEND EXTRA_RPATH ${PATH_TO_NAP_ROOT}/lib/${FBXCONVERTER_PACKAGED_BUILD_TYPE})

    set_single_config_installed_rpath_on_macos_object_for_dependent_modules(${FBXCONVERTER_PACKAGED_BUILD_TYPE} 
                                                                            "${DEPENDENT_NAP_MODULES}" 
                                                                            ${CMAKE_INSTALL_PREFIX}/tools/platform/fbxconverter
                                                                            "../.."
                                                                            "${EXTRA_RPATH}")
elseif(UNIX)
    set_installed_rpath_on_linux_object_for_dependent_modules("${DEPENDENT_NAP_MODULES}" ${PROJECT_NAME} "../..")
endif()

# Add tclap license into release
install(FILES ${THIRDPARTY_DIR}/tclap-1.2.1/README
              ${THIRDPARTY_DIR}/tclap-1.2.1/COPYING
        DESTINATION thirdparty/tclap)
install(DIRECTORY ${TCLAP_INCLUDE_DIRS}/tclap/ DESTINATION thirdparty/tclap/include/tclap)