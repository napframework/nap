# Some shared Qt configuration that should go in before add_library or add_executable


function(add_qt QT_MODULES)
    if (MSVC)
        find_path(QT_DIR
                NAMES Qt5/Qt5Config.cmake
                HINTS
                ${CMAKE_CURRENT_LIST_DIR}/../Qt/5.9.1/msvc2015_64/lib/cmake
                )
        set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${QT_DIR})
    endif()

    set(CMAKE_AUTOMOC ON)

    foreach(QT_MOD ${QT_MODULES})
        find_package(Qt5${QT_MOD} REQUIRED)
    endforeach()
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endfunction()

function(qtpost QT_MODULES)

    foreach (QT_MOD ${QT_MODULES})
        get_target_property(LOC Qt5::${QT_MOD} LOCATION_${CMAKE_BUILD_TYPE})
        file(COPY ${LOC} DESTINATION ${CMAKE_BINARY_DIR})
    endforeach ()

    get_target_property(LOC Qt5::Core LOCATION_${CMAKE_BUILD_TYPE})
    get_filename_component(DLL_DIR ${LOC} PATH)

    # Copy some other dlls
    set(MODULES icu*;libgcc_s_dw2-1;libstdc++-6;libwinpthread-1)
    foreach (MOD ${MODULES})
        file(GLOB MODULEFILES ${DLL_DIR}/${MOD}.dll)
        foreach (MODULEFILE ${MODULEFILES})
            file(COPY ${MODULEFILE} DESTINATION ${BIN_DIR})
        endforeach ()
    endforeach ()

    # Copy the binary resources to the bin dir so the executable can load them.
    file(GLOB RESOURCES resources/*.*)
    foreach (RES ${RESOURCES})
    file(COPY ${RES} DESTINATION ${BIN_DIR}/resources)
    endforeach()

    qt5_use_modules(${PROJECT_NAME} ${QT_MODULES})
endfunction()