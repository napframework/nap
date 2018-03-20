if(WIN32)
    # Copy etherdream DLL to project build directory
    if(NOT TARGET etherdreamlib)
        find_package(etherdream REQUIRED)
    endif()
    copy_etherdream_dll()
endif()