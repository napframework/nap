project(mod_kalvertoren)

# add all cpp files to SOURCES
file(GLOB SOURCES src/*.cpp)
file(GLOB HEADERS src/*.h)
source_group("headers" FILES ${HEADERS})
source_group("sources" FILES ${SOURCES})

# Get our NAP modules dependencies from module.json
module_json_to_cmake()

# compile shared lib as target
add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Apps)

# add include dirs
target_include_directories(${PROJECT_NAME} PUBLIC src)

# link with external libs
target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY _USE_MATH_DEFINES)
target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} napcore ${DEPENDENT_NAP_MODULES} nrender)

# Deploy module.json as MODULENAME.json alongside module post-build
copy_module_json_to_bin()

if (WIN32)
    set(FILES_TO_COPY
            ${THIRDPARTY_DIR}/assimp/msvc64/install/bin/assimp-vc140-mt.dll
            )

    copy_files_to_bin(${FILES_TO_COPY})
endif()
