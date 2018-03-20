if(WIN32)
    # Copy artnet DLL to project build directory
    if(NOT TARGET artnet)
        find_package(artnet REQUIRED)
    endif()
    copy_artnet_dll()
endif()