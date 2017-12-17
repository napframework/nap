project(unittests)

enable_testing()
find_package(Qt5Test REQUIRED)

file(GLOB SOURCES
        src/*.cpp
        src/*.hpp
        src/*.h)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME}
        napcore
        napkin_lib
        Qt5::Test
        mod_naprender
        nrender
        mod_napinput
        mod_napsdlinput
        mod_napsandbox
        #        mod_napmath
        #        mod_napsdlwindow
        #        mod_napapp
        )

# First copy some assets
copy_dir_to_bin(${CMAKE_CURRENT_LIST_DIR}/data data)

## Now run the tests right after the build
#add_custom_command(
#        TARGET ${PROJECT_NAME}
#        COMMENT "Run tests"
#        POST_BUILD
#        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
#        COMMAND ${PROJECT_NAME}
#)
