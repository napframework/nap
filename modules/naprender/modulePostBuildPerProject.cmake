if(WIN32)
    # Copy freeimage DLL to project build directory
    find_package(freeimage REQUIRED)
    copy_freeimage_dll()

    # Copy over Windows graphics DLLs to project build directory
    copy_base_windows_graphics_dlls()

    # Copy over Assimp to project build directory
    copy_files_to_bin(${THIRDPARTY_DIR}/assimp/msvc64/install/bin/assimp-vc140-mt.dll)
endif()