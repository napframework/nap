project(mod_napkalvertoren)

# add all cpp files to SOURCES
file(GLOB SOURCES src/*.cpp)
file(GLOB HEADERS src/*.h)
file(GLOB_RECURSE YOCTO_SRC src/yoctopuce/*.h src/yoctopuce/*.cpp src/yoctopuce/*.c)
source_group("headers" FILES ${HEADERS})
source_group("sources" FILES ${SOURCES})
source_group("yoctopuce" FILES ${YOCTO_SRC})

# LIBRARY

# compile shared lib as target
add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS} ${YOCTO_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Apps)

# add include dirs
target_include_directories(${PROJECT_NAME} PUBLIC src src/yoctopuce)

# link with external libs
target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY _USE_MATH_DEFINES)
target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} napcore mod_naprender nrender mod_napmath mod_napsdlinput mod_napsdlwindow mod_napcameracontrol mod_napapp mod_napartnet mod_napimgui)

if (WIN32)
    set(FILES_TO_COPY
            ${THIRDPARTY_DIR}/assimp/msvc64/install/bin/assimp-vc140-mt.dll
            )

    copy_files_to_bin(${FILES_TO_COPY})
endif()
