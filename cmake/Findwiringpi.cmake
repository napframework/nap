# - Try to find WiringPi

find_path(WIRINGPI_DIR
          NAMES
          linux/${ARCH}/include/wiringPi.h
          HINTS
          ${THIRDPARTY_DIR}/wiringpi
          ${CMAKE_CURRENT_LIST_DIR}/../../wiringpi
          )

if(UNIX)
    set(WIRINGPI_INCLUDE_DIR ${WIRINGPI_DIR}/linux/${ARCH}/include)
    set(WIRINGPI_LIBRARY_DIR ${WIRINGPI_DIR}/linux/${ARCH}/lib)
    set(WIRINGPI_LIBRARIES_RELEASE ${WIRINGPI_LIBRARY_DIR}/libwiringPi.so)
    set(WIRINGPI_LIBRARIES_DEBUG ${WIRINGPI_LIBRARY_DIR}/libwiringPi.so)
endif()

set(WIRINGPI_DIST_FILES ${WIRINGPI_DIR}/source/README.md)

include(FindPackageHandleStandardArgs)

# handle the QUIETLY and REQUIRED arguments and set WIRINGPI_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(wiringpi REQUIRED_VARS
        WIRINGPI_DIR
        WIRINGPI_LIBRARY_DIR
        WIRINGPI_INCLUDE_DIR
        WIRINGPI_LIBRARIES_RELEASE
        WIRINGPI_LIBRARIES_DEBUG
        WIRINGPI_DIST_FILES)
