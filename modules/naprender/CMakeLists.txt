project(mod_naprender)

# add all cpp files to SOURCES
file(GLOB SOURCES src/*.cpp)
file(GLOB HEADERS src/*.h)
source_group("headers" FILES ${HEADERS})
source_group("sources" FILES ${SOURCES})

# Get our NAP modules dependencies from module.json
module_json_to_cmake()

# LIBRARY

# compile shared lib as target
add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)

# add include dirs
target_include_directories(${PROJECT_NAME} PUBLIC ${NRENDER_INCLUDE_DIRS} src)

# link with external libs
target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY _USE_MATH_DEFINES)
target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} nrender ${DEPENDENT_NAP_MODULES})

# Deploy module.json as MODULENAME.json alongside module post-build
copy_module_json_to_bin()

# Package into platform release
if (UNIX)
    set(UNIX_EXTRA_RPATH ../../../../thirdparty/glew/lib)
    list(APPEND UNIX_EXTRA_RPATH ../../../../thirdparty/SDL2/lib)
    list(APPEND UNIX_EXTRA_RPATH ../../../../thirdparty/assimp/lib)
    list(APPEND UNIX_EXTRA_RPATH ../../../../thirdparty/FreeImage/lib)
endif()
package_module()

if (WIN32)
    # Install for fbxconverter
    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION tools/platform
            CONFIGURATIONS Release)
endif()
