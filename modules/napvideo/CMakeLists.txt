project(mod_napvideo)

# add all cpp files to SOURCES
file(GLOB SOURCES src/*.cpp)
file(GLOB HEADERS src/*.h)
source_group("headers" FILES ${HEADERS})
source_group("sources" FILES ${SOURCES})

find_package(FFmpeg REQUIRED)

add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)

target_include_directories(${PROJECT_NAME} PUBLIC src ${FFMPEG_INCLUDE_DIR})

target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)

set(DEPENDENT_NAP_MODULES mod_naprender)
target_link_libraries(${PROJECT_NAME} napcore ${DEPENDENT_NAP_MODULES} ${FFMPEG_LIBRARIES})

# Package into platform release
package_module()

# Set packaged Linux RPATH 
if (UNIX AND NOT APPLE)
    set(EXTRA_RPATH "../../../../thirdparty/FFmpeg/lib")
    set_installed_module_rpath_for_dependent_modules("${DEPENDENT_NAP_MODULES}" ${PROJECT_NAME} "${EXTRA_RPATH}")
endif()

# Package dependent includes 
# TODO discuss moving elsewhere, the idea here was that the object using the dependencies installs them

# FFmpeg
if(WIN32)
    install(DIRECTORY "${THIRDPARTY_DIR}/ffmpeg/" DESTINATION "thirdparty/FFmpeg"
            PATTERN "osx" EXCLUDE
            PATTERN "linux" EXCLUDE)
elseif(APPLE)
    # TODO Package FFmpeg license files

    # Install FFmpeg into release
    install(DIRECTORY "${THIRDPARTY_DIR}/ffmpeg/osx/install/" DESTINATION "thirdparty/FFmpeg")

    # Add RPATH for FFmpeg to packaged module on macOS
    foreach(build_type Release Debug)
        install(CODE "execute_process(COMMAND ${CMAKE_INSTALL_NAME_TOOL}
                                              -add_rpath
                                              @loader_path/../../../../thirdparty/FFmpeg/lib
                                              ${CMAKE_INSTALL_PREFIX}/modules/mod_napvideo/lib/${build_type}/libmod_napvideo.dylib                                           
                                      ERROR_QUIET
                                      )")
    endforeach()
elseif(UNIX)
    # TODO Package FFmpeg license files
    
    # Install FFmpeg into release
    install(DIRECTORY "${THIRDPARTY_DIR}/ffmpeg/linux/install/" DESTINATION "thirdparty/FFmpeg")

    # Let our installed FFmpeg libs find each other
    install(CODE "file(GLOB FFMPEG_DYLIBS ${CMAKE_INSTALL_PREFIX}/thirdparty/FFmpeg/lib/lib*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
                  foreach(FFMPEG_DYLIB \${FFMPEG_DYLIBS})
                      execute_process(COMMAND patchelf --set-rpath \$ORIGIN/. \${FFMPEG_DYLIB})
                  endforeach()
                  ")
endif()
