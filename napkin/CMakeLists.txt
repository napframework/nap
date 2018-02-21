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

set(LIBS
    napcore
    mod_napscene
    mod_nappython
    mod_napmath
    mod_naprender
    mod_napvideo
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


add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${CMAKE_CURRENT_LIST_DIR}/resources
                   $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources
                   COMMENT "Copy Resources")
