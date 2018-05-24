project(mod_napimgui)

# add all cpp files to SOURCES
file(GLOB SOURCES src/*.cpp)
file(GLOB HEADERS src/*.h)
file(GLOB_RECURSE GUI src/imgui/*.cpp src/imgui/*.h)

source_group("headers" FILES ${HEADERS})
source_group("sources" FILES ${SOURCES})
source_group("imgui" FILES ${GUI})

# Get our NAP modules dependencies from module.json
module_json_to_cmake()

# compile shared lib as target
add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS} ${GUI})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)

# add include dirs
target_include_directories(${PROJECT_NAME} PUBLIC src)

# preprocessors
target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)

# link libs
target_link_libraries(${PROJECT_NAME} ${DEPENDENT_NAP_MODULES})

# Deploy module.json as MODULENAME.json alongside module post-build
copy_module_json_to_bin()

# Package into platform release
if(UNIX)
    set(UNIX_EXTRA_RPATH "../../../../thirdparty/glew/lib")
    list(APPEND UNIX_EXTRA_RPATH "../../../../thirdparty/SDL2/lib")
endif()    
package_module()