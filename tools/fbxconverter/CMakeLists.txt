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
set(DEPENDENT_NAP_MODULES naprender napmath napscene)
target_link_libraries(${PROJECT_NAME} napcore ${DEPENDENT_NAP_MODULES})

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
                   )

# auto run test
#add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#        COMMAND ctest -C $<CONFIGURATION> --output-on-failure
#        DEPENDS always_rebuild)

# ==================================

# Package into NAP release
set(FBXCONVERTER_PACKAGED_BUILD_TYPE Release)
set(FBXCONVERTER_INSTALL_LOCATION tools/buildsystem)

install(TARGETS ${PROJECT_NAME}
        DESTINATION ${FBXCONVERTER_INSTALL_LOCATION}
        CONFIGURATIONS ${FBXCONVERTER_PACKAGED_BUILD_TYPE})

# Pull in deep module list, third party RPATHs
module_list_to_deep_dependencies_cmake("${DEPENDENT_NAP_MODULES}")

if (WIN32)
    file(GLOB ASSIMP_DLLS ${NAP_ROOT}/system_modules/naprender/thirdparty/assimp/msvc/x86_64/bin/*.dll)

    # TODO Update to find the libraries then install locating them via TARGET_FILE
    install(FILES ${NAP_ROOT}/system_modules/naprender/thirdparty/FreeImage/msvc/x86_64/Dist/x64/FreeImage.dll
                  ${THIRDPARTY_DIR}/rttr/msvc/x86_64/bin/rttr_core.dll
                  ${NAP_ROOT}/system_modules/naprender/thirdparty/sdl/msvc/x86_64/bin/SDL3.dll
                  ${NAP_ROOT}/system_modules/napfont/thirdparty/freetype/msvc/x86_64/lib/Release/freetype.dll
                  ${ASSIMP_DLLS}
            DESTINATION ${FBXCONVERTER_INSTALL_LOCATION})

    if(PACKAGE_PDBS)
        install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}>
                DESTINATION ${FBXCONVERTER_INSTALL_LOCATION}
                CONFIGURATIONS ${FBXCONVERTER_PACKAGED_BUILD_TYPE}
                )
    endif()
elseif(UNIX)
    set_installed_rpath_on_linux_object_for_dependent_modules("${DEEP_DEPENDENT_NAP_MODULES}"
                                                              ${PROJECT_NAME} "../.."
                                                              "${DEEP_DEPENDENT_RPATHS}"
                                                              )
endif()

# Add tclap into release
install(DIRECTORY ${TCLAP_DIR} DESTINATION thirdparty)
