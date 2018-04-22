project(mod_napetherdream)

# add all cpp files to SOURCES
file(GLOB_RECURSE SOURCES src/*.cpp src/*.h)
filter_platform_specific_files(SOURCES)
add_platform_specific_files("${WIN32_SOURCES}" "${MACOS_SOURCES}" "${LINUX_SOURCES}")

find_package(etherdream REQUIRED)

# LIBRARY

# compile shared lib as target
add_library(${PROJECT_NAME} SHARED ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)

# add include dirs
target_include_directories(${PROJECT_NAME} PUBLIC src ${ETHERDREAM_INCLUDE_DIRS})

# link with external libs
target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)
target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME})

set(DEPENDENT_NAP_MODULES mod_naprender mod_napmath mod_napscene)
target_link_libraries(${PROJECT_NAME} ${DEPENDENT_NAP_MODULES} etherdreamlib)

if(WIN32)
    # Copy etherdream DLL to build directory on Windows
    if(NOT TARGET etherdreamlib)
        find_package(etherdream REQUIRED)
    endif()
    copy_etherdream_dll()
endif()

# Package into platform release
if(UNIX)
    set(UNIX_EXTRA_RPATH ../../../../thirdparty/etherdream/bin)
endif()
package_module()

# Package etherdreamlib into platform release
install(FILES $<TARGET_FILE:etherdreamlib> DESTINATION thirdparty/etherdream/bin)
# Package etherdreamlib headers
install(DIRECTORY ${ETHERDREAM_INCLUDE_DIRS}/ DESTINATION thirdparty/etherdream/include)    
if(WIN32)
    # Package etherdream IMPLIB on Windows
    get_target_property(ETHERDREAM_IMPLIB etherdreamlib IMPORTED_IMPLIB_RELEASE)
    install(FILES ${ETHERDREAM_IMPLIB} DESTINATION thirdparty/etherdream/bin)    

endif()
