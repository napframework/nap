project(mod_napyoctopuce)

# add all cpp files to SOURCES
file(GLOB_RECURSE SOURCES src/*.cpp src/*.h)

find_package(yoctopuce REQUIRED)

# LIBRARY

# compile shared lib as target
add_library(${PROJECT_NAME} SHARED ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)

# add include dirs
target_include_directories(${PROJECT_NAME} PUBLIC src ${YOCTO_INCLUDE_DIRS})

# preprocessor
target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)

# link with external libs
if(NOT WIN32)
	target_compile_definitions(${PROJECT_NAME} PUBLIC HAVE_CONFIG_H)
endif()

set(DEPENDENT_NAP_MODULES napcore mod_napmath mod_napscene)
target_link_libraries(${PROJECT_NAME} ${DEPENDENT_NAP_MODULES} yoctopuce)

if (WIN32)
  copy_yoctopuce_dll()
endif()

# Package into platform release
if(UNIX)
    set(UNIX_EXTRA_RPATH "../../../../thirdparty/yoctopuce/bin")
endif()
package_module()

# Package yoctopuce into platform release
if(WIN32)
    install(FILES ${THIRDPARTY_DIR}/yoctopuce/Binaries/windows/x64/Release/yocto.dll DESTINATION thirdparty/yoctopuce/bin/release)
    install(FILES ${THIRDPARTY_DIR}/yoctopuce/Binaries/windows/x64/Release/yocto.lib DESTINATION thirdparty/yoctopuce/bin/release)
    install(FILES ${THIRDPARTY_DIR}/yoctopuce/Binaries/windows/x64/Debug/yocto.dll DESTINATION thirdparty/yoctopuce/bin/debug)
    install(FILES ${THIRDPARTY_DIR}/yoctopuce/Binaries/windows/x64/Debug/yocto.lib DESTINATION thirdparty/yoctopuce/bin/debug)
elseif(APPLE)
    install(FILES ${THIRDPARTY_DIR}/yoctopuce/Binaries/osx/libyocto.dylib DESTINATION thirdparty/yoctopuce/bin)
else()
    install(FILES ${THIRDPARTY_DIR}/yoctopuce/Binaries/linux/64bits/libyocto.so.1.0.1 DESTINATION thirdparty/yoctopuce/bin)
endif()
