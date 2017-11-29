cmake_minimum_required(VERSION 3.5)
project(@PROJECT_NAME_LOWERCASE@)

set(NAP_MODULES @MODULE_LIST_SUB_CMAKE@)

include(../../cmake/distmacros.cmake)
dist_project_template()
