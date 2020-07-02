# Exclude for Android
if(ANDROID)
    return()
endif()

project(napkin)

nap_qt_pre()

file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE HEADERS src/*.h)
file(GLOB_RECURSE RESOURCES resources/*.qrc)

if (MSVC)
    set(RESOURCES ${RESOURCES} resources/resources.rc)
endif()

add_executable(${PROJECT_NAME} ${SOURCES} ${UI_HEADERS} ${HEADERS} ${RESOURCES})
# TODO CH Review need to directly use generator expressions here, otherwise merge macOS and Win64
if(MSVC)
    if(NOT DEFINED NAP_PACKAGED_BUILD)
        set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${NAP_ROOT}/bin/${CMAKE_CXX_COMPILER_ID}-${ARCH}-$<CONFIG>/napkin)
        set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${NAP_ROOT}/bin/${CMAKE_CXX_COMPILER_ID}-${ARCH}-$<CONFIG>/napkin)
    else()
        set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${NAP_ROOT}/packaging_bin/${CMAKE_CXX_COMPILER_ID}-${ARCH}-$<CONFIG>/napkin)
        set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${NAP_ROOT}/packaging_bin/${CMAKE_CXX_COMPILER_ID}-${ARCH}-$<CONFIG>/napkin)
    endif()
else()
    if(NOT DEFINED NAP_PACKAGED_BUILD)
        if(APPLE)
# TODO CH Temporarily revert build conf to build type directory change
#           set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${NAP_ROOT}/bin/Release/napkin)
#           set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${NAP_ROOT}/bin/Debug/napkin)
            set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${NAP_ROOT}/bin/${BUILD_CONF}/napkin)
            set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${NAP_ROOT}/bin/${BUILD_CONF}/napkin)
        else()
# TODO CH Temporarily revert build conf to build type directory change
#           set(NAPKIN_OUTPUT_PATH ${NAP_ROOT}/bin/${CMAKE_BUILD_TYPE}/napkin)
            set(NAPKIN_OUTPUT_PATH ${NAP_ROOT}/bin/${BUILD_CONF}/napkin)
            set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${NAPKIN_OUTPUT_PATH})
            file(MAKE_DIRECTORY ${NAPKIN_OUTPUT_PATH})
        endif()
    endif()
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)")
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Tools)
target_include_directories(${PROJECT_NAME} PUBLIC src)


set(NAPKIN_LIB ${PROJECT_NAME}_lib)

# create library from app for unittests
add_library(${NAPKIN_LIB} STATIC ${SOURCES} ${UI_HEADERS} ${HEADERS} ${RESOURCES})

# The NAP modules Napkin needs to link against
# TODO this needs updating/verifying; can any of these move into NAP_MODULES_USAGE_DEPENDENCIES below? 
set(NAP_MODULES_LINK_DEPENDENCIES mod_napscene mod_nappython mod_napmath mod_naprender mod_napvideo mod_napaudio mod_napfont)

# This is the superset of modules that we want to have available to Napkin to run against; just a handy
# way to ensure they've built
set(NAP_MODULES_USAGE_DEPENDENCIES ${NAP_MODULES_LINK_DEPENDENCIES} mod_napinput mod_napcolor)

add_dependencies(${PROJECT_NAME} ${NAP_MODULES_USAGE_DEPENDENCIES})

set(LIBS
    napcore
    mod_napmath
    napqt
    ${NAP_MODULES_LINK_DEPENDENCIES}
    Qt5::Widgets Qt5::Core Qt5::Gui
    )

if(MSVC)
    list(APPEND UNITTEST_LIBS
         ${FREEIMAGE_LIBRARIES}
         ${FFMPEG_LIBRARIES}
         )
elseif(APPLE)
    list(APPEND UNITTEST_LIBS
         ${FREEIMAGE_LIBRARIES}
         ${FFMPEG_LIBRARIES}
         )

elseif(UNIX)
    list(APPEND UNITTEST_LIBS
         freeimage
         ffmpeg
         )
endif()

target_link_libraries(${PROJECT_NAME} ${LIBS} ${QT_LIBS})

target_link_libraries(${NAPKIN_LIB} ${LIBS})
target_include_directories(${NAPKIN_LIB} PUBLIC src ${FREEIMAGE_INCLUDE_DIRS})

nap_qt_post(${PROJECT_NAME})

# Add the runtime paths for RTTR & GLEW on macOS
if(APPLE)
    add_macos_rttr_rpath()
    add_macos_glew_rpath()
endif()

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${CMAKE_CURRENT_LIST_DIR}/resources
                   $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources
                   COMMENT "Copy Resources")

if(MSVC)
    if(DEFINED NAP_PACKAGED_BUILD)
        set(NAPKIN_COPY_SRC ${NAP_ROOT}/packaging_bin)
    else()
        set(NAPKIN_COPY_SRC ${NAP_ROOT}/bin)
    endif()

    # Deploy dependent modules on Win64
    foreach(MODULE_NAME ${NAP_MODULES_USAGE_DEPENDENCIES})
        add_custom_command(TARGET ${PROJECT_NAME}
                           POST_BUILD
                           COMMAND ${CMAKE_COMMAND} 
                                   -E copy
                                   $<TARGET_FILE:${MODULE_NAME}>
                                   $<TARGET_FILE_DIR:${PROJECT_NAME}>
                           )
    endforeach()

    # Deploy Qt plugins from thirdparty on Win64. Unlike macOS, Windows won't find 
    # the plugins under a plugins folder, the categories need to sit beside the binary.
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${QTDIR}/plugins/platforms $<TARGET_FILE_DIR:${PROJECT_NAME}>/platforms
        )

    # NAP RTTI
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:naprtti> $<TARGET_FILE_DIR:${PROJECT_NAME}>
        )

    # NAP core 
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:napcore> $<TARGET_FILE_DIR:${PROJECT_NAME}>
        )

    # Copy freeimage DLL to build directory
    if(NOT TARGET FreeImage)
        find_package(freeimage REQUIRED)
    endif()
    copy_files_to_target_file_dir($<TARGET_FILE:FreeImage>)

    # Copy over Windows graphics DLLs to build directory
    set(FILES_TO_COPY
        ${THIRDPARTY_DIR}/sdl2/msvc/lib/x64/SDL2.dll
        ${THIRDPARTY_DIR}/glew/msvc/bin/Release/x64/glew32.dll
        )
    copy_files_to_target_file_dir(${FILES_TO_COPY})

    # Copy over Assimp
    copy_files_to_target_file_dir(${THIRDPARTY_DIR}/assimp/msvc64/install/bin/assimp-vc140-mt.dll)

    # RTTR
    find_rttr()
    copy_files_to_target_file_dir($<TARGET_FILE:RTTR::Core>)

    # Python
    file(GLOB PYTHON_DLLS ${THIRDPARTY_DIR}/python/msvc/python-embed-amd64/*.dll)
    copy_files_to_target_file_dir(${PYTHON_DLLS})

    # Transitional thirdparty DLL copying until source and framework release 
    # structures are aligned (and we can use a shared module_extra.cmake) ##

    # mod_napvideo
    file(GLOB FFMPEGDLLS ${THIRDPARTY_DIR}/ffmpeg/msvc/install/bin/*.dll)
    copy_files_to_target_file_dir(${FFMPEGDLLS})

    # mod_napfont
    if(NOT TARGET freetype)
        find_package(freetype REQUIRED)
    endif()
    copy_files_to_target_file_dir($<TARGET_FILE:freetype>)

    # mod_napaudio
    if(NOT DEFINED LIBSNDFILE_LIB_DIR)
        find_package(libsndfile REQUIRED)
    endif()
    if(NOT DEFINED PORTAUDIO_LIB_DIR)
        find_package(portaudio REQUIRED)
    endif()
    if(NOT DEFINED LIBMPG123_LIB_DIR)
        find_package(libmpg123 REQUIRED)
    endif()
    set(FILES_TO_COPY
        ${LIBSNDFILE_LIB_DIR}/libsndfile-1.dll
        ${PORTAUDIO_LIB_DIR}/portaudio_x64.dll
        ${LIBMPG123_LIB_DIR}/libmpg123.dll
        )
    copy_files_to_target_file_dir(${FILES_TO_COPY})

    ########################################################################

endif()               

# Package into release build

# Package Qt into release
package_qt()

set(NAPKIN_PACKAGED_BUILD_TYPE Release)

include(${NAP_ROOT}/cmake/packaging_macros.cmake)

if(MSVC)
    # On Windows, for now, dump our whole napkin directory from packaging_bin
    # into the release as it contains the required DLLs (as we loaded them in 
    # there for running Napkin against source from its own directory)
    install(DIRECTORY ${NAP_ROOT}/packaging_bin/${CMAKE_CXX_COMPILER_ID}-${ARCH}-Release/napkin
            DESTINATION tools/)
else()
    # Deploy Napkin binary into release on *nix
    install(TARGETS ${PROJECT_NAME}
            DESTINATION tools/napkin
            CONFIGURATIONS Release)
endif()

if(APPLE)
    # Remove thirdparty from framework release at install time
    macos_remove_rpaths_from_object_at_install_time(${CMAKE_INSTALL_PREFIX}/tools/napkin/napkin
                                                    ${THIRDPARTY_DIR}  
                                                    Release)
endif()
if(MSVC AND PACKAGE_PDBS)
    install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION tools/napkin)
endif()        

install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/resources
        DESTINATION tools/napkin/)

set(NAPKIN_QT_INSTALL_FRAMEWORKS QtCore QtGui QtWidgets QtOpenGL)

set(PATH_TO_THIRDPARTY "../../thirdparty")
if(APPLE)
    list(APPEND NAPKIN_QT_INSTALL_FRAMEWORKS QtDBus)

    set(EXTRA_RPATH ${PATH_TO_THIRDPARTY}/assimp/lib)
    list(APPEND EXTRA_RPATH ${PATH_TO_THIRDPARTY}/glew/lib)
    list(APPEND EXTRA_RPATH ${PATH_TO_THIRDPARTY}/SDL2/lib)
    list(APPEND EXTRA_RPATH ${PATH_TO_THIRDPARTY}/Qt/lib)
    list(APPEND EXTRA_RPATH ${PATH_TO_THIRDPARTY}/FFmpeg/lib)
    list(APPEND EXTRA_RPATH ${PATH_TO_THIRDPARTY}/mpg123/lib)
    list(APPEND EXTRA_RPATH lib)

    # A temporary ugly fix for inter-dependent modules and their RPATHs on macOS.  NAP-225.
    list(APPEND EXTRA_RPATH ${PATH_TO_THIRDPARTY}/FreeImage/lib)
    list(APPEND EXTRA_RPATH ${PATH_TO_THIRDPARTY}/portaudio/lib)
    list(APPEND EXTRA_RPATH ${PATH_TO_THIRDPARTY}/libsndfile/lib)
    list(APPEND EXTRA_RPATH ${PATH_TO_THIRDPARTY}/freetype/lib)

    foreach(MODULECONFIG Release)
        set_single_config_installed_rpath_on_macos_object_for_dependent_modules(${MODULECONFIG} 
                                                                                "${NAP_MODULES_USAGE_DEPENDENCIES}" 
                                                                                ${CMAKE_INSTALL_PREFIX}/tools/napkin/napkin
                                                                                "../.."
                                                                                "${EXTRA_RPATH}")
    endforeach()

    # Install platform plugin alongside Napkin as it doesn't appear to be able to sourced
    # remotely (from thirdparty)
    install(DIRECTORY ${CMAKE_INSTALL_PREFIX}/thirdparty/Qt/plugins
            DESTINATION tools/napkin/
            CONFIGURATIONS Release
            )

    # Update paths to Qt frameworks in Napkin. Using explicit paths in an attempt to avoid loading
    # any installed Qt library.
    macos_replace_qt_framework_links_install_time("${NAPKIN_QT_INSTALL_FRAMEWORKS}" 
                                                  UNUSED
                                                  ${CMAKE_INSTALL_PREFIX}/tools/napkin/napkin
                                                  "@rpath"
                                                  )

#   ######### TODO CH review the need for an equivalent of these in Napkin relocation ############

#   set(PATH_TO_NAP_ROOT "@executable_path/../../../..")

#   # Add core libs path to RPATH
#   add_custom_command(TARGET ${PROJECT_NAME}
#                      POST_BUILD
#                      COMMAND ${CMAKE_INSTALL_NAME_TOOL} 
#                              -add_rpath ${PATH_TO_NAP_ROOT}/lib/Release
#                              ${CMAKE_INSTALL_PREFIX}/tools/napkin/napkin
#                      )

#   # Add module paths to RPATH
#   foreach(MODULE_NAME ${NAPKIN_DEPENDENT_NAP_MODULES})
#       add_custom_command(TARGET ${PROJECT_NAME}
#                          POST_BUILD
#                          COMMAND ${CMAKE_INSTALL_NAME_TOOL} 
#                                  -add_rpath ${PATH_TO_NAP_ROOT}/modules/${MODULE_NAME}/lib/Release
#                                  ${CMAKE_INSTALL_PREFIX}/tools/napkin/napkin
#                          )
#   endforeach()

#   ###########################################################################################

elseif(UNIX)
    if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
        set(EXTRA_RPATH ${PATH_TO_THIRDPARTY}/assimp/lib)
        list(APPEND EXTRA_RPATH ${PATH_TO_THIRDPARTY}/FreeImage/lib)
        list(APPEND EXTRA_RPATH ${PATH_TO_THIRDPARTY}/Qt/lib)
        list(APPEND EXTRA_RPATH ${PATH_TO_THIRDPARTY}/FFmpeg/lib)
        list(APPEND EXTRA_RPATH ${PATH_TO_THIRDPARTY}/mpg123/lib)
        list(APPEND EXTRA_RPATH ${PATH_TO_THIRDPARTY}/SDL2/lib)
        list(APPEND EXTRA_RPATH ${PATH_TO_THIRDPARTY}/glew/lib)
        list(APPEND EXTRA_RPATH lib)

        set_installed_rpath_on_linux_object_for_dependent_modules("${NAP_MODULES_USAGE_DEPENDENCIES}" ${PROJECT_NAME} "../.." "${EXTRA_RPATH}")

        # Install platform plugin alongside Napkin as it doesn't appear to be able to sourced
        # remotely (from thirdparty)
        install(FILES ${CMAKE_INSTALL_PREFIX}/thirdparty/Qt/plugins/platforms/libqxcb.so
                DESTINATION tools/napkin/platforms/
                PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
                )
    endif()
endif()
