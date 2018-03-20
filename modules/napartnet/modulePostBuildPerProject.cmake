if(WIN32)
    # Copy artnet DLL to project build directory
    find_package(artnet REQUIRED)
    copy_artnet_dll()
endif()