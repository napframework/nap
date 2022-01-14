# - Try to find WiringPi

# check for appropriate architecture
if(NOT ${ARCH} MATCHES "armhf")
    return()
endif()

find_path(WIRINGPI_DIR
          NO_CMAKE_FIND_ROOT_PATH
          NAMES include/wiringPi.h
          HINTS ${THIRDPARTY_DIR}/wiringpi
         )

set(WIRINGPI_INCLUDE_DIRS ${WIRINGPI_DIR}/include)
set(WIRINGPI_LIBS_DIR ${WIRINGPI_DIR}/lib)
set(WIRINGPI_LIBS_RELEASE ${WIRINGPI_LIBS_DIR}/libwiringPi.so)
set(WIRINGPI_LIBS_DEBUG ${WIRINGPI_LIBS_RELEASE})

mark_as_advanced(WIRINGPI_LIBS_DIR)

# promote package for find
INCLUDE(FindPackageHandleStandardArgs)
find_package_handle_standard_args(wiringpi REQUIRED_VARS WIRINGPI_DIR)

add_library(wiringpi SHARED IMPORTED)
set_target_properties(wiringpi PROPERTIES
                      IMPORTED_CONFIGURATIONS "Debug;Release"
                      IMPORTED_LOCATION_RELEASE ${WIRINGPI_LIBS_RELEASE}
                      IMPORTED_LOCATION_DEBUG ${WIRINGPI_LIBS_DEBUG}
                      )
