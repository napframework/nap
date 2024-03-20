project(napkin)
nap_qt_pre()

file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE HEADERS src/*.h)
file(GLOB_RECURSE RESOURCES resources/*.qrc)

if (MSVC)
    set(RESOURCES ${RESOURCES} resources/resources.rc)
endif()

add_executable(${PROJECT_NAME} ${SOURCES} ${UI_HEADERS} ${HEADERS} ${RESOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC src)

set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)")
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Tools)

if(WIN32)
    set_property(TARGET ${PROJECT_NAME} PROPERTY WIN32_EXECUTABLE true)
endif()

if(${ARCH} STREQUAL "armhf")
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -z muldefs")
endif()

# The NAP modules Napkin needs
set(NAP_MODULES_DEPENDENCIES napcore napscene napcolor napmath naprender napinput napfont)

# Add python if enabled
if(NAP_ENABLE_PYTHON)
    list(APPEND NAP_MODULES_DEPENDENCIES nappython)
endif()

# Set libraries
set(LIBS
    napqt
    ${NAP_MODULES_DEPENDENCIES}
    Qt6::Widgets Qt6::Core Qt6::Gui
    )

target_link_libraries(${PROJECT_NAME} ${LIBS} ${QT_LIBS})

nap_qt_post(${PROJECT_NAME})

# Add the runtime paths for RTTR
if(UNIX)
    file(RELATIVE_PATH rpath ${BIN_DIR} ${LIB_DIR})
    add_custom_command(TARGET ${PROJECT_NAME}
            POST_BUILD COMMAND
            ${CMAKE_INSTALL_NAME_TOOL} -add_rpath "@executable_path/${rpath}/."
            $<TARGET_FILE:${PROJECT_NAME}>)
endif()

# Copy resources
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${CMAKE_CURRENT_LIST_DIR}/resources
                   ${DATA_DIR}/resources
                   COMMENT "Copy Resources")

# Copy Qt DLLs to bin
add_custom_command(TARGET ${PROJECTNAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:Qt6::Widgets>
        $<TARGET_FILE:Qt6::Core>
        $<TARGET_FILE:Qt6::Gui>
        $<TARGET_FILE:Qt6::OpenGL>
        OUTPUT ${LIB_DIR}
        COMMENT "Copy Qt DLLs")

# Deploy Qt plugins from thirdparty on Win64. Unlike macOS, Windows won't find
# the plugins under a plugins folder, the categories need to sit beside the binary.
add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${QTDIR}/plugins/platforms ${LIB_DIR}/platforms
)
