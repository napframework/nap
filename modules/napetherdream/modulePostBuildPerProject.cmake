if(WIN32)
    # Copy etherdream DLL to project build directory
    find_package(etherdream REQUIRED)
    copy_etherdream_dll()
endif()