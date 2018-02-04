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


set(DEPENDENT_NAP_MODULES mod_napscene mod_nappython mod_napmath)

set(LIBS
    napcore
    ${DEPENDENT_NAP_MODULES}
    Qt5::Widgets Qt5::Core Qt5::Gui
    )

target_link_libraries(${PROJECT_NAME} ${LIBS} ${QT_LIBS})

target_link_libraries(${NAPKIN_LIB} ${LIBS})
target_include_directories(${NAPKIN_LIB} PUBLIC src)

nap_qt_post(${PROJECT_NAME})

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${CMAKE_CURRENT_LIST_DIR}/resources
                   $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources
                   COMMENT "Copy Resources")

# Package into release build
set(NAPKIN_PACKAGED_BUILD_TYPE Release)
install(TARGETS ${PROJECT_NAME} DESTINATION tools
        CONFIGURATIONS ${NAPKIN_PACKAGED_BUILD_TYPE})

# TODO use get_prerequisites or find_package to make this a whole load less terrible?

if (WIN32)
    install(FILES "../../../thirdparty/assimp/msvc64/install/bin/assimp-vc140-mt.dll" DESTINATION "tools/")
    install(FILES "../../../thirdparty/FreeImage/msvc/Dist/x64/FreeImage.dll" DESTINATION "tools/")
    install(FILES "../../../thirdparty/glew/msvc/bin/Release/x64/glew32.dll" DESTINATION "tools/")
    install(FILES "../../../thirdparty/rttr/msvc64/install/bin/rttr_core.dll" DESTINATION "tools/")
    install(FILES "../../../thirdparty/SDL2/msvc/lib/x64/SDL2.dll" DESTINATION "tools/")
elseif(APPLE)
    set(BUILT_RPATH "@executable_path/../thirdparty/rttr/bin/")
    set(BUILT_RPATH "${BUILT_RPATH};@executable_path/../lib/${NAPKIN_PACKAGED_BUILD_TYPE}/")
    foreach(MODULE_NAME ${DEPENDENT_NAP_MODULES})
        set(BUILT_RPATH "${BUILT_RPATH};@executable_path/../modules/${MODULE_NAME}/lib/${NAPKIN_PACKAGED_BUILD_TYPE}/")
    endforeach()
    set_target_properties(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "${BUILT_RPATH}")
elseif(UNIX)
    set_installed_linux_object_for_dependent_modules("${DEPENDENT_NAP_MODULES}" ${PROJECT_NAME} "..")

    # Add RTTR
    get_target_property(BUILT_RPATH ${PROJECT_NAME} INSTALL_RPATH)
    set(BUILT_RPATH "${BUILT_RPATH}:$ORIGIN/../thirdparty/rttr/bin/")
    set_target_properties(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "${BUILT_RPATH}")
endif()
