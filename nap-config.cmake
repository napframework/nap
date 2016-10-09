 get_filename_component(SELF_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
# include(${SELF_DIR}/core/CMakeLists.txt)
 get_filename_component(nap_INCLUDE_DIRS "${SELF_DIR}/core/src" ABSOLUTE)
 include_directories(${nap_INCLUDE_DIRS})
 