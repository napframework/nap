project(mod_naprender)

# add all cpp files to SOURCES
file(GLOB SOURCES src/*.cpp)
file(GLOB HEADERS src/*.h)
source_group("headers" FILES ${HEADERS})
source_group("sources" FILES ${SOURCES})

# LIBRARY

# compile shared lib as target
add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)

# add include dirs
target_include_directories(${PROJECT_NAME} PUBLIC ${NRENDER_INCLUDE_DIRS} src)

# link with external libs
target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY _USE_MATH_DEFINES)
target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} nrender mod_napmath mod_napscene)

if (WIN32)
    copy_files_to_bin(${THIRDPARTY_DIR}/assimp/msvc64/install/bin/assimp-vc140-mt.dll)

    copy_base_windows_graphics_dlls()

    find_package(freeimage REQUIRED)
    copy_freeimage_dll()
endif()
