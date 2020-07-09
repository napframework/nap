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
endif()

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${CMAKE_CURRENT_LIST_DIR}/resources
                   $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources
                   COMMENT "Copy Resources")

# Package into release build
set(NAPKIN_PACKAGED_BUILD_TYPE Release)

include(${NAP_ROOT}/cmake/packaging_macros.cmake)

install(TARGETS ${PROJECT_NAME}
        DESTINATION tools/platform/napkin/Release
        CONFIGURATIONS Release)
install(TARGETS ${PROJECT_NAME}
        DESTINATION tools/platform/napkin/Debug
        CONFIGURATIONS Debug)
if(APPLE)
    # Remove thirdparty from framework release at install time
    macos_remove_rpaths_from_object_at_install_time(${CMAKE_INSTALL_PREFIX}/tools/platform/napkin/Release/napkin
                                                    ${THIRDPARTY_DIR}  
                                                    Release)
    macos_remove_rpaths_from_object_at_install_time(${CMAKE_INSTALL_PREFIX}/tools/platform/napkin/Debug/napkin
                                                    ${THIRDPARTY_DIR}  
                                                    Debug)
endif()
if(MSVC AND PACKAGE_PDBS)
    install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION tools/platform/napkin/$<CONFIG>)
endif()        

install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/resources
        DESTINATION tools/platform/napkin/)

set(NAPKIN_QT_INSTALL_FRAMEWORKS QtCore QtGui QtWidgets QtOpenGL)

set(PATH_TO_THIRDPARTY "../../../../thirdparty")
if(APPLE)
    set(EXTRA_RPATH ${PATH_TO_THIRDPARTY}/assimp/lib)
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
    list(APPEND EXTRA_RPATH ${PATH_TO_THIRDPARTY}/vulkansdk/lib)

    foreach(MODULECONFIG Release Debug)
        set_single_config_installed_rpath_on_macos_object_for_dependent_modules(${MODULECONFIG} 
                                                                                "${NAP_MODULES_USAGE_DEPENDENCIES}" 
                                                                                ${CMAKE_INSTALL_PREFIX}/tools/platform/napkin/${MODULECONFIG}/napkin
                                                                                "../../../.."
                                                                                "${EXTRA_RPATH}")
    endforeach()

elseif(UNIX)
    set(EXTRA_RPATH ${PATH_TO_THIRDPARTY}/assimp/lib)
    list(APPEND EXTRA_RPATH ${PATH_TO_THIRDPARTY}/FreeImage/lib)
    list(APPEND EXTRA_RPATH ${PATH_TO_THIRDPARTY}/Qt/lib)
    list(APPEND EXTRA_RPATH ${PATH_TO_THIRDPARTY}/FFmpeg/lib)
    list(APPEND EXTRA_RPATH ${PATH_TO_THIRDPARTY}/mpg123/lib)
    list(APPEND EXTRA_RPATH ${PATH_TO_THIRDPARTY}/SDL2/lib)
    list(APPEND EXTRA_RPATH ${PATH_TO_THIRDPARTY}/vulkansdk/lib)
    list(APPEND EXTRA_RPATH lib)

    set_installed_rpath_on_linux_object_for_dependent_modules("${NAP_MODULES_USAGE_DEPENDENCIES}" ${PROJECT_NAME} "../../../.." "${EXTRA_RPATH}")
endif()
