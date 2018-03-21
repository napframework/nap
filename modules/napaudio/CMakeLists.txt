project(mod_napaudio)

find_package(portaudio REQUIRED)
find_package(libsndfile REQUIRED)
find_package(libmpg123 REQUIRED)

file(GLOB_RECURSE sources src/*.cpp src/*.h)

source_group("core" src/audio/core/*.*)
source_group("node" src/audio/node/*.*)
source_group("component" src/audio/component/*.*)
source_group("resource" src/audio/resource/*.*)
source_group("service" src/audio/service/*.*)
source_group("utility" src/audio/utility/*.*)

set(LIBRARIES napcore portaudio libsndfile libmpg123)
if(APPLE)
    list(APPEND LIBRARIES "-framework CoreAudio" "-framework CoreServices" "-framework CoreFoundation" "-framework AudioUnit" "-framework AudioToolbox")
endif()

set(INCLUDES src ${PORTAUDIO_INCLUDE_DIR} ${LIBSNDFILE_INCLUDE_DIR} ${LIBMPG123_INCLUDE_DIR})

add_library(${PROJECT_NAME} SHARED ${sources})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)

target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDES})
set(DEPENDENT_NAP_MODULES mod_napmath mod_napscene)
target_link_libraries(${PROJECT_NAME} ${LIBRARIES} ${DEPENDENT_NAP_MODULES})

target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)
target_compile_definitions(${PROJECT_NAME} PRIVATE _USE_MATH_DEFINES)

# Copy over DLLs on Windows
if(WIN32)
    if(NOT DEFINED LIBSNDFILE_LIB_DIR)
        find_package(libsndfile REQUIRED)
    endif()

    if(NOT DEFINED PORTAUDIO_LIB_DIR)
        find_package(portaudio REQUIRED)
    endif()

    if(NOT DEFINED LIBMPG123_LIB_DIR)
        find_package(libmpg123 REQUIRED)
    endif()

    # Copy audio DLLs to project build directory
    set(FILES_TO_COPY
        ${LIBSNDFILE_LIB_DIR}/libsndfile-1.dll
        ${PORTAUDIO_LIB_DIR}/portaudio_x64.dll
        ${LIBMPG123_LIB_DIR}/libmpg123.dll
    )

    copy_files_to_bin(${FILES_TO_COPY})
endif()

# Package into platform release
if(UNIX)
    set(UNIX_EXTRA_RPATH "../../../../thirdparty/mpg123/lib")
    if(NOT APPLE)
        list(APPEND UNIX_EXTRA_RPATH "../../../../thirdparty/libsndfile/lib")
        list(APPEND UNIX_EXTRA_RPATH "../../../../thirdparty/portaudio/lib")
    endif()
endif()    
package_module()

# Package mpg123 into platform release
install(FILES ${THIRDPARTY_DIR}/mpg123/doc/LICENSE DESTINATION thirdparty/mpg123)
if(WIN32)
    file(GLOB MPG123_DYLIBS ${THIRDPARTY_DIR}/mpg123/install/msvc/*.dll)
    install(FILES ${MPG123_DYLIBS}
            DESTINATION thirdparty/mpg123/bin)
    file(GLOB MPG123_HEADERS ${THIRDPARTY_DIR}/mpg123/install/msvc/*.h)
    install(FILES ${MPG123_HEADERS}
            DESTINATION thirdparty/mpg123/include)
    file(GLOB MPG123_LIBFILES ${THIRDPARTY_DIR}/mpg123/install/msvc/*.lib)
    install(FILES ${MPG123_LIBFILES}
            DESTINATION thirdparty/mpg123/lib)   
elseif(APPLE)
    # Ensure our mpg123 install name id is set properly, this is really for intall into packaging
    add_custom_command(TARGET ${PROJECT_NAME}
                       PRE_BUILD
                       COMMAND ${CMAKE_INSTALL_NAME_TOOL} -id @rpath/libmpg123.dylib ${LIBMPG123_LIB_DIR}/libmpg123.0.dylib
                       )

    file(GLOB MPG123_DYLIBS ${LIBMPG123_LIB_DIR}/libmpg*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
    install(FILES ${MPG123_DYLIBS}
            DESTINATION thirdparty/mpg123/lib)

    foreach(build_type Release Debug)
        ensure_macos_file_has_rpath_at_install(${CMAKE_INSTALL_PREFIX}/modules/mod_napaudio/lib/${build_type}/libmod_napaudio.dylib
                                               "@loader_path/../../../../thirdparty/mpg123/lib")
    endforeach()
elseif(UNIX)
    file(GLOB MPG123_DYLIBS ${LIBMPG123_LIB_DIR}/libmpg123*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
    install(FILES ${MPG123_DYLIBS}
            DESTINATION thirdparty/mpg123/lib)
endif()

# Package libsndfile into platform release
install(FILES ${THIRDPARTY_DIR}/libsndfile/linux/install/COPYING DESTINATION thirdparty/libsndfile)
if(WIN32)
    install(DIRECTORY ${THIRDPARTY_DIR}/libsndfile/msvc64/
            DESTINATION thirdparty/libsndfile)
elseif(UNIX AND NOT APPLE)
    file(GLOB LIBSNDFILE_DYLIBS ${LIBSNDFILE_LIB_DIR}/libsnd*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
    install(FILES ${LIBSNDFILE_DYLIBS}
            DESTINATION thirdparty/libsndfile/lib)
endif()

# Package portaudio into platform release
install(FILES ${THIRDPARTY_DIR}/portaudio/linux/install/LICENSE.txt DESTINATION thirdparty/portaudio)
if(WIN32)
    install(DIRECTORY ${THIRDPARTY_DIR}/portaudio/msvc64/
            DESTINATION thirdparty/portaudio)
elseif(UNIX AND NOT APPLE)
    file(GLOB PORTAUDIO_DYLIBS ${PORTAUDIO_LIB_DIR}/libport*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
    install(FILES ${PORTAUDIO_DYLIBS}
            DESTINATION thirdparty/portaudio/lib)
endif()