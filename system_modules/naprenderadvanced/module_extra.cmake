if (NOT NAP_BUILD_CONTEXT MATCHES "source")
    # Install data directory into packaged app
    install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/data DESTINATION system_modules/naprenderadvanced)
endif()

# Check existence of bcm_host.h header file to see if we're building on Raspberry
if(${ARCH} MATCHES "armhf")
    MESSAGE(VERBOSE "Looking for bcm_host.h")
    INCLUDE(CheckIncludeFiles)

    # Raspbian bullseye bcm_host.h location
    CHECK_INCLUDE_FILES("/usr/include/bcm_host.h" RENDERADVANCED_RASPBERRYPI)

    # otherwise, check previous location of bcm_host.h on older Raspbian OS's
    if(NOT RENDERADVANCED_RASPBERRYPI)
        CHECK_INCLUDE_FILES("/opt/vc/include/bcm_host.h" RENDERADVANCED_RASPBERRYPI)
    endif()
endif()

if(RENDERADVANCED_RASPBERRYPI)
    target_compile_definitions(${PROJECT_NAME} PRIVATE RENDERADVANCED_RPI)
endif()
