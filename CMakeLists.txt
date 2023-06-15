cmake_minimum_required(VERSION 3.18.4)
project(NAP)

# Keep the current directory as project root
set(NAP_ROOT ${CMAKE_CURRENT_LIST_DIR})
message(STATUS "Using NAP Root: ${CMAKE_CURRENT_LIST_DIR}")

set(NAP_BUILD_CONTEXT "source")

# Thirdparty libraries reside in this location, keep an absolute path
set(THIRDPARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty)
message(STATUS "Using thirdparty directory: ${THIRDPARTY_DIR}")

include(cmake/macros_and_functions.cmake)
include(cmake/framework_release_packaging.cmake)
bootstrap_environment()

set_source_build_configuration()

include(cmake/qtconfig.cmake)

set(MODULE_INTO_PARENT TRUE)

set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/packaging_staging)

# store project directory
get_filename_component(PROJECTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/.. ABSOLUTE)

# library targets
add_subdirectory(core)
add_subdirectory(utility)
add_subdirectory(rtti)
add_subdirectory(qt)

# tools targets
add_subdirectory(tools/fbxconverter)
add_subdirectory(tools/napkin)
add_subdirectory(tools/keygen)
add_subdirectory(tools/licensegenerator)

# START_SOURCE_ARCHIVE_REMOVED_SECTION (don't remove)
# app targets, add your own app targets here!
add_subdirectory(apps/example)
# END_SOURCE_ARCHIVE_REMOVED_SECTION (don't remove)

# demo targets
add_subdirectory(demos/helloworld)
add_subdirectory(demos/rotatingcube)
add_subdirectory(demos/vinyl)
add_subdirectory(demos/multiwindow)
add_subdirectory(demos/heightmap)
add_subdirectory(demos/dynamicgeo)
add_subdirectory(demos/computeparticles)
add_subdirectory(demos/computeflocking)
add_subdirectory(demos/videomodulation)
add_subdirectory(demos/blobtrace)
add_subdirectory(demos/audioanalysis)
add_subdirectory(demos/audioplayback)
add_subdirectory(demos/oscmidi)
add_subdirectory(demos/curveball)
add_subdirectory(demos/copystamp)
add_subdirectory(demos/websocketserver)
add_subdirectory(demos/sequencer)
add_subdirectory(demos/paintobject)
add_subdirectory(demos/licensecheck)
add_subdirectory(demos/artnetsend)
add_subdirectory(demos/artnetreceive)
add_subdirectory(demos/webportal)
add_subdirectory(demos/python)

if(APPLE)
    set(GENERATE_XCODE_PROJECT_TARGET generateXcodeProject)
    add_custom_target(generateXcodeProject ${CMAKE_COMMAND} -H. -B. -G Xcode -DCMAKE_BUILD_TYPE=Debug ../)
endif()
#add_dependencies(${PROJECT_NAME} ${GENERATE_XCODE_PROJECT_TARGET})

# Modules
add_subdirectory(system_modules/napinput)
add_subdirectory(system_modules/naprender)
add_subdirectory(system_modules/napmath)
add_subdirectory(system_modules/napsdlinput)
add_subdirectory(system_modules/napscene)
add_subdirectory(system_modules/napcameracontrol)
add_subdirectory(system_modules/nappython)
add_subdirectory(system_modules/napvideo)
add_subdirectory(system_modules/napaudio)
add_subdirectory(system_modules/napartnet)
add_subdirectory(system_modules/naposc)
add_subdirectory(system_modules/napsvg)
add_subdirectory(system_modules/napapp)
add_subdirectory(system_modules/napimgui)
add_subdirectory(system_modules/napmidi)
add_subdirectory(system_modules/napfont)
add_subdirectory(system_modules/napapi)
add_subdirectory(system_modules/napparameter)
add_subdirectory(system_modules/napparametergui)
add_subdirectory(system_modules/napserial)
add_subdirectory(system_modules/napwebsocket)
add_subdirectory(system_modules/napapiwebsocket)
add_subdirectory(system_modules/napcolor)
add_subdirectory(system_modules/napsequence)
add_subdirectory(system_modules/napsequencegui)
add_subdirectory(system_modules/napsequenceaudio)
add_subdirectory(system_modules/napsequenceaudiogui)
add_subdirectory(system_modules/naplicense)
add_subdirectory(system_modules/napudp)
add_subdirectory(system_modules/napcalendar)
add_subdirectory(system_modules/napasio)
add_subdirectory(system_modules/napportal)

solution_info_to_cmake()

# Add additional directories if specified from command line.
# For example: -DADDITIONAL_SUB_DIRECTORIES="apps/myapp;apps/myapp/module"
if(ADDITIONAL_SUB_DIRECTORIES)
    foreach(SUB_DIR ${ADDITIONAL_SUB_DIRECTORIES})
        message(STATUS "Adding sub directory: ${SUB_DIR}")
        add_subdirectory(${SUB_DIR})
    endforeach()
endif()


# Packaging
include(cmake/framework_release_packaging.cmake)
package_nap()
