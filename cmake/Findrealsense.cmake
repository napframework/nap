# - Try to find RealSense SDK

find_path(REALSENSE_DIR
          NAMES
          source/rs.h
          HINTS
          ${THIRDPARTY_DIR}/realsense
          ${CMAKE_CURRENT_LIST_DIR}/../../realsense
          )

set(REALSENSE_INCLUDE_DIR ${REALSENSE_DIR}/source)
set(REALSENSE_LIBRARY_DIR ${REALSENSE_DIR}/linux/${ARCH}/lib)
set(REALSENSE_LIBRARIES_RELEASE ${REALSENSE_LIBRARY_DIR}/librealsense2.so.2.51)
set(REALSENSE_LIBRARIES_DEBUG ${REALSENSE_LIBRARY_DIR}/librealsense2.so.2.51)

#set(WIRINGPI_DIST_FILES ${WIRINGPI_DIR}/source/README.md)

include(FindPackageHandleStandardArgs)

# handle the QUIETLY and REQUIRED arguments and set WIRINGPI_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(realsense REQUIRED_VARS
        REALSENSE_DIR
        REALSENSE_INCLUDE_DIR
        REALSENSE_LIBRARY_DIR
        REALSENSE_LIBRARIES_DEBUG
        REALSENSE_LIBRARIES_DEBUG)
        #WIRINGPI_DIST_FILES)
