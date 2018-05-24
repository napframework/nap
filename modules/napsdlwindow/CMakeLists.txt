project(mod_napsdlwindow)

# add all cpp files to SOURCES
file(GLOB SOURCES src/*.cpp)
file(GLOB HEADERS src/*.h)
source_group("headers" FILES ${HEADERS})
source_group("sources" FILES ${SOURCES})

# Get our NAP modules dependencies from module.json
module_json_to_cmake()

if(WIN32)
    if(MSVC)
        set(CMAKE_LIBRARY_PATH ${THIRDPARTY_DIR}/SDL2/msvc/lib/x64)
        set(CMAKE_PREFIX_PATH ${THIRDPARTY_DIR}/SDL2/msvc)
    else()
        set(CMAKE_LIBRARY_PATH ${THIRDPARTY_DIR}/SDL2/mingw/x86_64-w64-mingw32)
        set(CMAKE_PREFIX_PATH ${THIRDPARTY_DIR}/SDL2/mingw/x86_64-w64-mingw32)
    endif()
elseif(APPLE)
    set(CMAKE_LIBRARY_PATH ${THIRDPARTY_DIR}/SDL2/osx/install/lib)
    set(CMAKE_PREFIX_PATH ${THIRDPARTY_DIR}/SDL2/osx/install)
elseif(UNIX)
    set(CMAKE_LIBRARY_PATH ${THIRDPARTY_DIR}/SDL2/linux/install/lib)
    set(CMAKE_PREFIX_PATH ${THIRDPARTY_DIR}/SDL2/linux/install)    
endif()
find_package(SDL2 REQUIRED)

# compile shared lib as target
add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)

# add include dirs
target_include_directories(${PROJECT_NAME} PUBLIC src ${SDL2_INCLUDE_DIR})
target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)
target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME})

# link with external libs
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARY} napcore)

# Deploy module.json as MODULENAME.json alongside module post-build
copy_module_json_to_bin()

# Package into platform release
if(UNIX)
    set(UNIX_EXTRA_RPATH ../../../../thirdparty/SDL2/lib)
    # Workaround for module not having dependent modules but wanting to set extra RPATH
    set(DEPENDENT_NAP_MODULES mod_napsdlwindow)
endif()    
package_module()