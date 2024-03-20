cmake_minimum_required(VERSION 3.19)
set(CMAKE_OSX_ARCHITECTURES arm64;x86_64)
project(NAP)

set(NAP_ROOT ${CMAKE_CURRENT_LIST_DIR})
include(${NAP_ROOT}/cmake/setup.cmake)
setup()

add_subdirectory(utility)
add_subdirectory(rtti)
add_subdirectory(core)
add_subdirectory(system_modules/napmath)
add_subdirectory(system_modules/napscene)
add_subdirectory(system_modules/napfont)
add_subdirectory(system_modules/napinput)
add_subdirectory(system_modules/napcolor)
add_subdirectory(system_modules/naprender)
add_subdirectory(system_modules/napsdlinput)
add_subdirectory(system_modules/napapp)
add_subdirectory(system_modules/napimgui)
add_subdirectory(system_modules/napparameter)
add_subdirectory(system_modules/napcameracontrol)

add_subdirectory(qt)

add_subdirectory(demos/helloworld)
add_subdirectory(tools/fbxconverter)
add_subdirectory(tools/napkin)

