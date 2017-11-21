cmake_minimum_required(VERSION 3.5)
project(rendertest)
set(NAP_MODULES mod_naprender mod_napmath mod_napinput mod_napsdlinput mod_napsdlwindow mod_napapp mod_nappython mod_napcameracontrol mod_napscene)

include(../../cmake/distmacros.cmake)
dist_project_template()
