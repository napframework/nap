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

target_link_libraries(${PROJECT_NAME} napcore mod_naprender nrender mod_napmath)


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
install(TARGETS ${PROJECT_NAME} DESTINATION tools)
