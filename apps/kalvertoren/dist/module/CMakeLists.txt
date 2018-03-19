cmake_minimum_required(VERSION 3.5)
set(DEPENDENT_MODULES mod_naprender mod_napmath mod_napsdlinput mod_napsdlwindow mod_napcameracontrol mod_napapp mod_napartnet mod_napimgui mod_napscene)
set(NAP_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
include(${NAP_ROOT}/cmake/nap_module.cmake)
