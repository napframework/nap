set(NAP_MAJOR_VERSION 0)
set(NAP_MINOR_VERSION 4)
set(NAP_PATCH_VERSION 5)
set(NAP_VERSION "${NAP_MAJOR_VERSION}.${NAP_MINOR_VERSION}.${NAP_PATCH_VERSION}")
# Be careful updating this as it's currently parsed during packaging by package.py
message(STATUS "NAP version: ${NAP_VERSION}")