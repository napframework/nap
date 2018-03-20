project(unittests)

enable_testing()
nap_qt_pre()

file(GLOB SOURCES
     src/*.cpp
     src/*.hpp
     src/*.h)

add_executable(${PROJECT_NAME} ${SOURCES})
add_executable(${PROJECT_NAME}_autorun ${SOURCES})

set(NAP_MODULES mod_napkalvertoren mod_naprender mod_napinput mod_napsdlinput mod_napsdlwindow mod_napsandbox mod_napvideo mod_napmath mod_napscene mod_napartnet mod_napcameracontrol)

set(UNITTEST_LIBS
    napcore
    ${NAP_MODULES}
    napkin_lib
    nrender
    )

find_package(FFmpeg REQUIRED)

if (MSVC)
    list(APPEND UNITTEST_LIBS
            ${FREEIMAGE_LIBRARIES}
            ${FFMPEG_LIBRARIES}
            )
elseif (APPLE)
    list(APPEND UNITTEST_LIBS
            ${FREEIMAGE_LIBRARIES}
            ${FFMPEG_LIBRARIES}
            )

elseif (UNIX)
    list(APPEND UNITTEST_LIBS
            ${FREEIMAGE_LIBRARIES}  
            ${FFMPEG_LIBRARIES}
            )
endif ()

target_link_libraries(${PROJECT_NAME} ${UNITTEST_LIBS})
target_link_libraries(${PROJECT_NAME}_autorun ${UNITTEST_LIBS})

# First copy some assets
copy_dir_to_bin(${CMAKE_CURRENT_LIST_DIR}/data unit_tests_data)

nap_qt_post(${PROJECT_NAME})

# Set output paths, copy Windows DLLs, package into release, etc
nap_source_project_output_and_packaging(FALSE FALSE "unused" FALSE)

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
