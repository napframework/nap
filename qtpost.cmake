# Some shared Qt configuration that should go in after add_library or add_executable

# Copy qt dlls
set(QT_MODULES Core;Gui;Widgets;Xml)
foreach (QT_MOD ${QT_MODULES})
    get_target_property(LOC Qt5::${QT_MOD} LOCATION_${CMAKE_BUILD_TYPE})
    file(COPY ${LOC} DESTINATION ${BIN_DIR})
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
