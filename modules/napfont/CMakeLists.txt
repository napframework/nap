project(mod_napfont)

# add all cpp files to SOURCES
file(GLOB_RECURSE SOURCES src/*.cpp src/*.h)

# Get our NAP modules dependencies from module.json
module_json_to_cmake()

find_package(freetype REQUIRED)

# LIBRARY

# compile shared lib as target
add_library(${PROJECT_NAME} SHARED ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)

# add include dirs
target_include_directories(${PROJECT_NAME} PUBLIC src ${FREETYPE_INCLUDE_DIRS})

# preprocessor
target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)

# link with external libs
if(NOT WIN32)
	target_compile_definitions(${PROJECT_NAME} PUBLIC HAVE_CONFIG_H)
endif()
target_link_libraries(${PROJECT_NAME} ${DEPENDENT_NAP_MODULES} freetype)

# Copy freetype DLL
if (WIN32)
    copy_freetype_dll()
endif()

# Deploy module.json as MODULENAME.json alongside module post-build
copy_module_json_to_bin()

# Package into platform release
if(UNIX)
    set(UNIX_EXTRA_RPATH "../../../../thirdparty/freetype/bin")
endif()
package_module()

# Package freetype into platform release
file(GLOB FREETYPE_LICENSE_FILES ${THIRDPARTY_DIR}/freetype/docs/*.txt)
install(FILES ${FREETYPE_LICENSE_FILES} 
              ${THIRDPARTY_DIR}/freetype/README
  DESTINATION thirdparty/freetype)

install(DIRECTORY ${THIRDPARTY_DIR}/freetype/include/ 
        DESTINATION thirdparty/freetype/include
        FILES_MATCHING PATTERN "*.h")

if(WIN32)
    install(FILES ${THIRDPARTY_DIR}/freetype/msvc/Release/freetype.dll DESTINATION thirdparty/freetype/bin/release)
    install(FILES ${THIRDPARTY_DIR}/freetype/msvc/Release/freetype.lib DESTINATION thirdparty/freetype/bin/release)
    install(FILES ${THIRDPARTY_DIR}/freetype/msvc/Debug/freetyped.dll DESTINATION thirdparty/freetype/bin/debug)
    install(FILES ${THIRDPARTY_DIR}/freetype/msvc/Debug/freetyped.lib DESTINATION thirdparty/freetype/bin/debug)
elseif(APPLE)
    SET(FREETYPE_DYLIB ${THIRDPARTY_DIR}/freetype/osx/libfreetype.dylib)
    install(FILES ${FREETYPE_DYLIB} DESTINATION thirdparty/freetype/bin)

    # Ensure our freetype install name id is set properly, this is really for intall into packaging
    add_custom_command(TARGET ${PROJECT_NAME}
                       PRE_BUILD
                       COMMAND ${CMAKE_INSTALL_NAME_TOOL} -id @rpath/libfreetype.dylib ${FREETYPE_DYLIB}
                       COMMENT "Setting install name for freetype")

    foreach(build_type Release Debug)
        install(CODE "execute_process(COMMAND ${CMAKE_INSTALL_NAME_TOOL}
                                              -add_rpath
                                              @loader_path/../../../../thirdparty/freetype/bin
                                              ${CMAKE_INSTALL_PREFIX}/modules/mod_napfont/lib/${build_type}/mod_napfont.dylib
                                      ERROR_QUIET
                                      )")
    endforeach()    
else()
    install(FILES ${THIRDPARTY_DIR}/freetype/linux/libfreetype.so DESTINATION thirdparty/freetype/bin)
endif()

if (WIN32)
    # Install for fbxconverter
    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION tools/platform
        CONFIGURATIONS Release)
endif()
