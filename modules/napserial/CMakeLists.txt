# Exclude for Android
if(ANDROID)
    return()
endif()

project(mod_napserial)

# add all cpp files to SOURCES
file(GLOB_RECURSE SOURCES src/*.cpp src/*.h)

# Get our NAP modules dependencies from module.json
module_json_to_cmake()

find_package(serial REQUIRED)

# LIBRARY

# compile shared lib as target
add_library(${PROJECT_NAME} SHARED ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)

# add include dirs
target_include_directories(${PROJECT_NAME} PUBLIC src ${SERIAL_INCLUDE_DIRS})

# preprocessor
target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)

# link libs
target_link_libraries(${PROJECT_NAME} napcore ${DEPENDENT_NAP_MODULES} debug ${SERIAL_LIBS_DEBUG} optimized ${SERIAL_LIBS_RELEASE})

# Deploy module.json as MODULENAME.json alongside module post-build
copy_module_json_to_bin()

# Package into platform release
#if(APPLE)
#    set(MACOS_EXTRA_RPATH_RELEASE ../../../../thirdparty/oscpack/lib/Release)
#    set(MACOS_EXTRA_RPATH_DEBUG ../../../../thirdparty/oscpack/lib/Debug)
#elseif(UNIX)
#    set(LINUX_EXTRA_RPATH ../../../../thirdparty/oscpack/lib)
#endif()
package_module()

# Package oscpack into platform release
install(FILES ${THIRDPARTY_DIR}/serial/README.md DESTINATION thirdparty/serial)
install(DIRECTORY ${THIRDPARTY_DIR}/serial/include 
        DESTINATION thirdparty/serial
        PATTERN "*.cpp" EXCLUDE)
if(MSVC)
    install(DIRECTORY ${THIRDPARTY_DIR}/serial/msvc/x64 DESTINATION thirdparty/serial/lib)
elseif(APPLE)
    install(DIRECTORY ${THIRDPARTY_DIR}/serial/osx DESTINATION thirdparty/serial/lib)
else()
    #install(DIRECTORY ${THIRDPARTY_DIR}/oscpack/linux/install/lib/ DESTINATION thirdparty/oscpack/lib)
endif()
