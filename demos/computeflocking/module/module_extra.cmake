# Check existence of bcm_host.h header file to see if we're building on Raspberry
if(${ARCH} MATCHES "armhf")
    MESSAGE(VERBOSE "Looking for bcm_host.h")
    INCLUDE(CheckIncludeFiles)

    # Raspbian bullseye bcm_host.h location
    CHECK_INCLUDE_FILES("/usr/include/bcm_host.h" COMPUTEFLOCKING_RASPBERRY)

    # otherwise, check previous location of bcm_host.h on older Raspbian OS's
    if(NOT COMPUTEFLOCKING_RASPBERRY)
        CHECK_INCLUDE_FILES("/opt/vc/include/bcm_host.h" COMPUTEFLOCKING_RASPBERRY)
    endif()
endif()

if(COMPUTEFLOCKING_RASPBERRY)
    target_compile_definitions(${PROJECT_NAME} PRIVATE COMPUTEFLOCKING_RPI)
endif()
