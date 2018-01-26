project(unittests)

enable_testing()
nap_qt_pre()

file(GLOB SOURCES
     src/*.cpp
     src/*.hpp
     src/*.h)

add_executable(${PROJECT_NAME} ${SOURCES})
add_executable(${PROJECT_NAME}_autorun ${SOURCES})

set(UNITTEST_LIBS
    napcore
    mod_napkalvertoren
    napkin_lib
    mod_naprender
    nrender
    mod_napinput
    mod_napsdlinput
    mod_napsandbox
    #        mod_napmath
    #        mod_napsdlwindow
    #        mod_napapp
    )

target_link_libraries(${PROJECT_NAME} ${UNITTEST_LIBS})
target_link_libraries(${PROJECT_NAME}_autorun ${UNITTEST_LIBS})

# First copy some assets
copy_dir_to_bin(${CMAKE_CURRENT_LIST_DIR}/data data)

nap_qt_post(${PROJECT_NAME})

# Now run the tests right after the build
add_custom_command(TARGET ${PROJECT_NAME}_autorun
                   COMMENT "Run tests"
                   POST_BUILD
                   WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                   COMMAND $<TARGET_FILE:${PROJECT_NAME}>
                   )
