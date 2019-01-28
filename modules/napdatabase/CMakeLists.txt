project(mod_napdatabase)

# add all cpp files to SOURCES
file(GLOB_RECURSE SOURCES src/*.cpp src/*.h)
if(ANDROID)
    file(GLOB SQLITE ${THIRDPARTY_DIR}/sqlite/*.c ${THIRDPARTY_DIR}/sqlite/*.h)
endif()

# Get our NAP modules dependencies from module.json
module_json_to_cmake()


# LIBRARY

# compile shared lib as target
add_library(${PROJECT_NAME} SHARED ${SOURCES} ${SQLITE})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)

# add include dirs
target_include_directories(${PROJECT_NAME} PUBLIC src)
if(ANDROID)
    target_include_directories(${PROJECT_NAME} PUBLIC ${THIRDPARTY_DIR}/sqlite)
endif()

# preprocessor
target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)

# link with external libs
if(NOT WIN32)
	target_compile_definitions(${PROJECT_NAME} PUBLIC HAVE_CONFIG_H)
endif()

target_link_libraries(${PROJECT_NAME} ${DEPENDENT_NAP_MODULES} napcore)

# Use precompiled lib on Win64 for now
if(WIN32)
    find_package(sqlite REQUIRED)
    target_include_directories(${PROJECT_NAME} PUBLIC ${SQLITE_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} debug ${SQLITE_LIBS_DEBUG} optimized ${SQLITE_LIBS_RELEASE})
endif()

# Deploy module.json as MODULENAME.json alongside module post-build
copy_module_json_to_bin()

# Package into platform release
package_module()

# Package information 3rd party database should go here

# TODO TEMPORARY SQLite temporary packaging into release
if(ANDROID)
    install(DIRECTORY ${THIRDPARTY_DIR}/sqlite/ DESTINATION thirdparty/sqlite)
endif()
