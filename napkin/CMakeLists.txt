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
    # Copy all of our Windows DLLs that have build in bin into project dir
    # TODO this is NOT a clean solution, but if we're mainly releasing projects build against packaged releases of the
    #      framework maybe this is OK?
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

if(APPLE)
    # TODO Update this to the newer cleaner RPATH setting style like we're using on Linux
    set(path_to_nap_root "@executable_path/../../../..")
    set(BUILT_RPATH "${path_to_nap_root}/thirdparty/rttr/bin/")
    set(BUILT_RPATH "${BUILT_RPATH};${path_to_nap_root}/thirdparty/glew/lib/")
    set(BUILT_RPATH "${BUILT_RPATH};${path_to_nap_root}/thirdparty/assimp/lib/")
    set(BUILT_RPATH "${BUILT_RPATH};${path_to_nap_root}/thirdparty/SDL2/lib/")
    set(BUILT_RPATH "${BUILT_RPATH};${path_to_nap_root}/thirdparty/python/lib/")
    set(BUILT_RPATH "${BUILT_RPATH};${path_to_nap_root}/thirdparty/Qt/lib/")
    set(BUILT_RPATH "${BUILT_RPATH};${path_to_nap_root}/thirdparty/FFmpeg/lib/")
    set(BUILT_RPATH "${BUILT_RPATH};@executable_path/lib/")
    set_target_properties(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "${BUILT_RPATH}")
elseif(UNIX)
    set(EXTRA_RPATH "../../../../thirdparty/assimp/lib")
    list(APPEND EXTRA_RPATH "../../../../thirdparty/FreeImage/lib")
    list(APPEND EXTRA_RPATH "../../../../thirdparty/FFmpeg/lib")
    list(APPEND EXTRA_RPATH "../../../../thirdparty/Qt/lib")
    list(APPEND EXTRA_RPATH "lib")
    set_installed_rpath_on_linux_object_for_dependent_modules("${DEPENDENT_NAP_MODULES}" ${PROJECT_NAME} "../../../.." "${EXTRA_RPATH}")
endif()