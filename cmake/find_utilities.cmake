# Find RTTR using our thirdparty paths
function(find_rttr)
    if(NOT TARGET RTTR::Core)
        if(WIN32)
            set(RTTR_DIR "${THIRDPARTY_DIR}/rttr/msvc/x86_64/cmake")
        elseif(APPLE)
            set(RTTR_DIR "${THIRDPARTY_DIR}/rttr/macos/${ARCH}/cmake")
            find_path(
                    RTTR_DIR
                    NAMES rttr-config.cmake
                    HINTS
                    ${THIRDPARTY_DIR}/rttr/macos/${ARCH}/cmake
            )
        else()
            set(RTTR_DIR "${THIRDPARTY_DIR}/rttr/linux/${ARCH}/cmake")
        endif()
        find_package(RTTR CONFIG REQUIRED Core)
        set(RTTR_LICENSE_FILES ${RTTR_DIR}/)
    endif()
endfunction()