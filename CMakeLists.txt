cmake_minimum_required(VERSION 3.18.4)
project(NAP)

# Keep the current directory as project root
set(NAP_ROOT ${CMAKE_CURRENT_LIST_DIR})
message(STATUS "Using NAP Root: ${CMAKE_CURRENT_LIST_DIR}")

set(NAP_BUILD_CONTEXT "source")

# Thirdparty libraries reside in this location, keep an absolute path
get_filename_component(THIRDPARTY_DIR ../thirdparty REALPATH BASE_DIR ${CMAKE_CURRENT_LIST_DIR})
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
# executable targets
add_subdirectory(test/render)

# app targets, add your own app targets here!
add_subdirectory(apps/example)
add_subdirectory(apps/example/module)
# END_SOURCE_ARCHIVE_REMOVED_SECTION (don't remove)

# demo targets
add_subdirectory(demos/helloworld)
add_subdirectory(demos/vinyl)
add_subdirectory(demos/multiwindow)
add_subdirectory(demos/heightmap)
add_subdirectory(demos/heightmap/module)
add_subdirectory(demos/dynamicgeo)
add_subdirectory(demos/dynamicgeo/module)
add_subdirectory(demos/computeparticles)
add_subdirectory(demos/computeparticles/module)
add_subdirectory(demos/computeflocking)
add_subdirectory(demos/computeflocking/module)
add_subdirectory(demos/videomodulation)
add_subdirectory(demos/videomodulation/module)
add_subdirectory(demos/blobtrace)
add_subdirectory(demos/audioanalysis)
add_subdirectory(demos/audioplayback)
add_subdirectory(demos/oscmidi)
add_subdirectory(demos/oscmidi/module)
add_subdirectory(demos/lineblending)
add_subdirectory(demos/lineblending/module)
add_subdirectory(demos/curveball)
add_subdirectory(demos/curveball/module)
add_subdirectory(demos/copystamp)
add_subdirectory(demos/copystamp/module)
add_subdirectory(demos/websocketserver)
add_subdirectory(demos/websocketserver/module)
add_subdirectory(demos/facedetection)
add_subdirectory(demos/facedetection/module)
add_subdirectory(demos/sequencer)
add_subdirectory(demos/paintobject)
add_subdirectory(demos/licensecheck)
add_subdirectory(demos/tween)
add_subdirectory(demos/artnetsend)
add_subdirectory(demos/artnetreceive)
add_subdirectory(demos/artnetreceive/module)
add_subdirectory(demos/pipins)
add_subdirectory(demos/webportal)
add_subdirectory(demos/python)

if(APPLE)
    set(GENERATE_XCODE_PROJECT_TARGET generateXcodeProject)
    add_custom_target(generateXcodeProject cmake -H. -Bxcode -G Xcode -DCMAKE_BUILD_TYPE=Debug ../)
endif()
#add_dependencies(${PROJECT_NAME} ${GENERATE_XCODE_PROJECT_TARGET})

# Modules
add_subdirectory(modules/napinput)
add_subdirectory(modules/naprender)
add_subdirectory(modules/napmath)
add_subdirectory(modules/napsdlinput)
add_subdirectory(modules/napscene)
add_subdirectory(modules/napcameracontrol)
add_subdirectory(modules/nappython)
add_subdirectory(modules/napvideo)
add_subdirectory(modules/napetherdream)
add_subdirectory(modules/napaudio)
add_subdirectory(modules/napartnet)
add_subdirectory(modules/naposc)
add_subdirectory(modules/napsvg)
add_subdirectory(modules/napapp)
add_subdirectory(modules/napimgui)
add_subdirectory(modules/napmidi)
add_subdirectory(modules/napyoctopuce)
add_subdirectory(modules/napfont)
add_subdirectory(modules/napdatabase)
add_subdirectory(modules/napapi)
add_subdirectory(modules/napparameter)
add_subdirectory(modules/napparametergui)
add_subdirectory(modules/napserial)
add_subdirectory(modules/napwebsocket)
add_subdirectory(modules/napapiwebsocket)
add_subdirectory(modules/napopencv)
add_subdirectory(modules/napsoem)
add_subdirectory(modules/napcolor)
add_subdirectory(modules/napsequence)
add_subdirectory(modules/napsequencegui)
add_subdirectory(modules/napsequenceaudio)
add_subdirectory(modules/napsequenceaudiogui)
add_subdirectory(modules/naplicense)
add_subdirectory(modules/napudp)
add_subdirectory(modules/naptween)
add_subdirectory(modules/napcalendar)
add_subdirectory(modules/napasio)
add_subdirectory(modules/nappipins)
add_subdirectory(modules/napportal)

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
