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

set(DEPENDENT_NAP_MODULES mod_naprender mod_napmath)
target_link_libraries(${PROJECT_NAME} napcore nrender ${DEPENDENT_NAP_MODULES})

if (WIN32)
    # Copy freeimage DLL
    find_package(freeimage REQUIRED)
    copy_freeimage_dll()

    # Copy over some crap window dlls
    copy_base_windows_graphics_dlls()
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
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_LIST_DIR}/res/vinyl_cover.fbx
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/res-test/vinyl_cover.fbx
        DEPENDS always_rebuild
        )

# auto run test
#add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#        COMMAND ctest -C $<CONFIGURATION> --output-on-failure
#        DEPENDS always_rebuild)

# Package into release build
# TODO for now use debug build for Linux due to release build crash issues 28-11-17
if (WIN32 OR APPLE)
    install(TARGETS ${PROJECT_NAME} DESTINATION tools
            CONFIGURATIONS Release)
else()
    install(TARGETS ${PROJECT_NAME} DESTINATION tools
            CONFIGURATIONS Debug)
endif()

# install(CODE "execute_process(COMMAND python ../dist/osx/dylibpathfix.py fbxconverter_pathfix\nWORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/packaging)")

if (WIN32)
        # TODO use get_prerequisites or find_package to make this a whole load less terrible
        #install(CODE "include(../cmake/configure.cmake)\ninstall_binary_requisites(${CMAKE_SOURCE_DIR}/packaging/tools/fbxconverter.exe ${CMAKE_SOURCE_DIR}/packaging/tools)")

        install(FILES "../../../thirdparty/assimp/msvc64/install/bin/assimp-vc140-mt.dll" DESTINATION "tools/")
        install(FILES "../../../thirdparty/FreeImage/msvc/Dist/x64/FreeImage.dll" DESTINATION "tools/")
        install(FILES "../../../thirdparty/glew/msvc/bin/Release/x64/glew32.dll" DESTINATION "tools/")
        install(FILES "../../../thirdparty/rttr/msvc64/install/bin/rttr_core.dll" DESTINATION "tools/")
        install(FILES "../../../thirdparty/SDL2/msvc/lib/x64/SDL2.dll" DESTINATION "tools/")
elseif(UNIX AND NOT APPLE)
    set_installed_linux_object_for_dependent_modules("${DEPENDENT_NAP_MODULES}" ${PROJECT_NAME} "..")

    # TODO use get_prerequisites or find_package to make this a whole load less terrible

    # Add RTTR
    get_target_property(BUILT_RPATH ${PROJECT_NAME} INSTALL_RPATH)
    set(BUILT_RPATH "${BUILT_RPATH}:$ORIGIN/../thirdparty/rttr/bin/")
    set_target_properties(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "${BUILT_RPATH}")
endif()
