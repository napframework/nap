cmake_minimum_required(VERSION 3.19)
set(CMAKE_OSX_ARCHITECTURES arm64;x86_64)
project(NAP)

# Set global directories
set(NAP_ROOT ${CMAKE_CURRENT_LIST_DIR})
set(THIRDPARTY_DIR ${NAP_ROOT}/thirdparty)
set(BIN_DIR ${CMAKE_BINARY_DIR}/bin)
set(LIB_DIR ${BIN_DIR}/lib)

set(CMAKE_INSTALL_BINDIR Contents/MacOS)
set(CMAKE_INSTALL_LIBDIR Contents/MacOS/lib)
set(CMAKE_INSTALL_DATADIR Contents/Resources)

include(${NAP_ROOT}/cmake/setup.cmake)
setup()

add_subdirectory(apps/test)
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

add_subdirectory(demos/helloworld)
add_subdirectory(tools/fbxconverter)

