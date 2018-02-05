# Install executable
install(PROGRAMS ${NAP_ROOT}/tools/napkin
        DESTINATION .)
# Install resources
install(DIRECTORY ${NAP_ROOT}/tools/resources
        DESTINATION .)
# Install main QT libs from thirdparty
install(DIRECTORY ${THIRDPARTY_DIR}/QT/lib/
        DESTINATION lib)
# Install QT plugins from thirdparty
install(DIRECTORY ${THIRDPARTY_DIR}/QT/plugins
        DESTINATION .)

# Ensure we have our dependent modules
set(NAPKIN_MODULES mod_nappython mod_napmath mod_napscene)
set(INSTALLING_MODULE_FOR_NAPKIN TRUE)
foreach(NAP_MODULE ${NAPKIN_MODULES})
    find_nap_module(${NAP_MODULE})
endforeach()
unset(INSTALLING_MODULE_FOR_NAPKIN)

# RPATH work for macOS
if(APPLE)
    install(CODE "execute_process(COMMAND ${CMAKE_INSTALL_NAME_TOOL} 
                                          -add_rpath 
                                          @executable_path/lib
                                          ${CMAKE_INSTALL_PREFIX}/napkin
                                  ERROR_QUIET)")

    file(GLOB imageformat_plugins RELATIVE ${THIRDPARTY_DIR}/QT/plugins/imageformats "${THIRDPARTY_DIR}/QT/plugins/imageformats/*dylib")
    foreach(imageformat_plugin ${imageformat_plugins})
        install(CODE "execute_process(COMMAND ${CMAKE_INSTALL_NAME_TOOL} 
                                              -add_rpath 
                                              @loader_path/../../lib/
                                              ${CMAKE_INSTALL_PREFIX}/plugins/imageformats/${imageformat_plugin}
                                      ERROR_QUIET)")    
    endforeach()

    install(CODE "execute_process(COMMAND ${CMAKE_INSTALL_NAME_TOOL} 
                                          -add_rpath 
                                          @loader_path/../../lib/
                                          ${CMAKE_INSTALL_PREFIX}/plugins/platforms/libqcocoa.dylib
                                  ERROR_QUIET)")
endif()