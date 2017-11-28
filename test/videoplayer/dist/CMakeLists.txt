cmake_minimum_required(VERSION 3.5)
project(videoplayer)
set(NAP_MODULES mod_naprender mod_napmath mod_napinput mod_napsdlinput mod_napsdlwindow mod_napapp mod_napvideo mod_napscene mod_napimgui)

include(../../cmake/distmacros.cmake)
dist_project_template()
