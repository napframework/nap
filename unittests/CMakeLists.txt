project(unittests)
enable_testing()
nap_qt_pre()

file(GLOB_RECURSE SOURCES
     src/*.cpp
     src/*.hpp
     src/*.h)

add_executable(${PROJECT_NAME} ${SOURCES})
add_executable(${PROJECT_NAME}_autorun ${SOURCES})

set(UNITTEST_LIBS
    napcore
    napkin_lib
    mod_napaudio
    )

target_link_libraries(${PROJECT_NAME} ${UNITTEST_LIBS})
target_link_libraries(${PROJECT_NAME}_autorun ${UNITTEST_LIBS})

# First copy some assets
copy_dir_to_bin(${CMAKE_CURRENT_LIST_DIR}/unit_tests_data unit_tests_data)

nap_qt_post(${PROJECT_NAME})

# Package into release, export FBX, other shared source project fixes
nap_source_project_packaging_and_shared_postprocessing(FALSE FALSE "unused" FALSE)

# Now run the tests right after the build
#
# TODO These are currently disabling while packaging a platform release due to:
# - Modules locating etc path problems due to the fact that we have NAP_PACKAGED_BUILD set but have 
#   (close to) NAP source file structure
# - Further path issues due to specially named lib output dir for packaging
if(NOT NAP_PACKAGED_BUILD)
    add_custom_command(TARGET ${PROJECT_NAME}_autorun
                   COMMENT "Run tests"
                   POST_BUILD
                   WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                   COMMAND "$<TARGET_FILE:${PROJECT_NAME}>"
                   )
endif()
