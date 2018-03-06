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


set(DEPENDENT_NAP_MODULES mod_napscene mod_nappython mod_napmath mod_naprender mod_napvideo)

set(LIBS
    napcore
    ${DEPENDENT_NAP_MODULES}
    Qt5::Widgets Qt5::Core Qt5::Gui
    )

if(MSVC)
    list(APPEND UNITTEST_LIBS
         ${FREEIMAGE_LIBRARIES}
         ${FFMPEG_LIBRARIES}
         )
    copy_freeimage_dll()
    copy_windows_ffmpeg_dlls()
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

# Output to bin/napkin
set_output_directories()

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${CMAKE_CURRENT_LIST_DIR}/resources
                   $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources
                   COMMENT "Copy Resources")

if(WIN32)
    # Copy all of our Windows DLLs that have build in bin into project dir.  Temporary approach until we're
    # reading cleaner approach ala what we do against NAP release makes it into builds against NAP source.
    bulk_copy_windows_dlls_to_bin()
endif()

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