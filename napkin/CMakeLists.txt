project(napkin)

nap_qt_pre()

file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE HEADERS src/*.h)
file(GLOB_RECURSE RESOURCES resources/*.qrc)

add_executable(${PROJECT_NAME} ${SOURCES} ${UI_HEADERS} ${HEADERS} ${RESOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)")
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Tools)
target_include_directories(${PROJECT_NAME} PUBLIC src)


set(NAPKIN_LIB ${PROJECT_NAME}_lib)

# create library from app for unittests
add_library(${NAPKIN_LIB} STATIC ${SOURCES} ${UI_HEADERS} ${HEADERS} ${RESOURCES})

# The NAP modules Napkin needs to link against
# TODO this needs updating/verifying
set(NAP_MODULES_LINK_DEPENDENCIES mod_napscene mod_nappython mod_napmath mod_naprender mod_napvideo)

# This is the superset of modules that we want to have available to Napkin to run against
set(NAP_MODULES_USAGE_DEPENDENCIES 
    ${NAP_MODULES_LINK_DEPENDENCIES} 
    mod_napapp 
    mod_napartnet
    mod_napaudio
    mod_napaudioadvanced
    mod_napcameracontrol
    mod_napetherdream
    mod_napimgui
    mod_napinput
    mod_napmidi
    mod_naposc
    mod_napsandbox
    mod_napsdlinput
    mod_napsdlwindow
    mod_napsvg
    )

add_dependencies(${PROJECT_NAME} ${NAP_MODULES_USAGE_DEPENDENCIES})

set(LIBS
    napcore
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

# Set our NAP modules to be list of modules we want to use with Napkin so we they get copied into Napkin's
# binary directory on Windows
set(NAP_MODULES ${NAP_MODULES_USAGE_DEPENDENCIES})

# Set output paths, copy Windows DLLs, package into release, etc
nap_source_project_output_and_packaging(FALSE FALSE "unused" FALSE)

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
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/resources
        DESTINATION tools/platform/napkin/)

set(NAPKIN_QT_INSTALL_FRAMEWORKS QtCore QtGui QtWidgets)

set(PATH_TO_THIRDPARTY "../../../../thirdparty")
if(APPLE)
    set(EXTRA_RPATH ${PATH_TO_THIRDPARTY}/assimp/lib)
    list(APPEND EXTRA_RPATH ${PATH_TO_THIRDPARTY}/glew/lib)
    list(APPEND EXTRA_RPATH ${PATH_TO_THIRDPARTY}/SDL2/lib)
    list(APPEND EXTRA_RPATH ${PATH_TO_THIRDPARTY}/Qt/lib)
    list(APPEND EXTRA_RPATH ${PATH_TO_THIRDPARTY}/FFmpeg/lib)
    list(APPEND EXTRA_RPATH lib)

    foreach(MODULECONFIG Release Debug)
        set_single_config_installed_rpath_on_macos_object_for_dependent_modules(${MODULECONFIG} 
                                                                                "${DEPENDENT_NAP_MODULES}" 
                                                                                ${CMAKE_INSTALL_PREFIX}/tools/platform/napkin/${MODULECONFIG}/napkin
                                                                                "../../../.."
                                                                                "${EXTRA_RPATH}")
    endforeach()

elseif(UNIX)
    set(EXTRA_RPATH ${PATH_TO_THIRDPARTY}/assimp/lib)
    list(APPEND EXTRA_RPATH ${PATH_TO_THIRDPARTY}/FreeImage/lib)
    list(APPEND EXTRA_RPATH ${PATH_TO_THIRDPARTY}/FFmpeg/lib)
    list(APPEND EXTRA_RPATH ${PATH_TO_THIRDPARTY}/Qt/lib)
    list(APPEND EXTRA_RPATH lib)

    set_installed_rpath_on_linux_object_for_dependent_modules("${DEPENDENT_NAP_MODULES}" ${PROJECT_NAME} "../../../.." "${EXTRA_RPATH}")
endif()