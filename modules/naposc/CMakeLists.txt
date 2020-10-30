project(mod_naposc)

# add all cpp files to SOURCES
file(GLOB_RECURSE SOURCES src/*.cpp src/*.h)

# Get our NAP modules dependencies from module.json
module_json_to_cmake()

find_package(oscpack REQUIRED)

# LIBRARY

# compile shared lib as target
add_library(${PROJECT_NAME} SHARED ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)
# Remove lib prefix on Unix libraries
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

# add include dirs
target_include_directories(${PROJECT_NAME} PUBLIC src ${OSCPACK_INCLUDE_DIRS})

# preprocessor
target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)

# link libs
target_link_libraries(${PROJECT_NAME} napcore ${DEPENDENT_NAP_MODULES} debug ${OSCPACK_LIBS_DEBUG} optimized ${OSCPACK_LIBS_RELEASE})

# Deploy module.json as MODULENAME.json alongside module post-build
copy_module_json_to_bin()

# Package into platform release
if(APPLE)
    set(MACOS_EXTRA_RPATH_RELEASE ../../../../thirdparty/oscpack/lib/Release)
    set(MACOS_EXTRA_RPATH_DEBUG ../../../../thirdparty/oscpack/lib/Debug)
elseif(UNIX)
    set(LINUX_EXTRA_RPATH ../../../../thirdparty/oscpack/lib)
endif()
package_module()

# Package oscpack into platform release
install(FILES ${THIRDPARTY_DIR}/oscpack/LICENSE DESTINATION thirdparty/oscpack)
install(FILES ${THIRDPARTY_DIR}/oscpack/README DESTINATION thirdparty/oscpack)
install(DIRECTORY ${THIRDPARTY_DIR}/oscpack/osc 
                  ${THIRDPARTY_DIR}/oscpack/ip 
        DESTINATION thirdparty/oscpack/include
        PATTERN "*.cpp" EXCLUDE)
if(MSVC)
    install(DIRECTORY ${THIRDPARTY_DIR}/oscpack/msvc64/ DESTINATION thirdparty/oscpack/lib)
elseif(APPLE)
    install(DIRECTORY ${THIRDPARTY_DIR}/oscpack/osx/install/lib/ DESTINATION thirdparty/oscpack/lib)
elseif(ANDROID)
    install(DIRECTORY ${THIRDPARTY_DIR}/oscpack/android/install/lib/ DESTINATION thirdparty/oscpack/lib)
else()
    install(DIRECTORY ${THIRDPARTY_DIR}/oscpack/linux/install/lib/ DESTINATION thirdparty/oscpack/lib)
endif()
