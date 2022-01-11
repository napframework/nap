cmake_minimum_required(VERSION 3.18.4)

project(mod_napwiringpi)

find_package(wiringpi REQUIRED)

# add all cpp files to SOURCES
file(GLOB_RECURSE SOURCES src/*.cpp src/*.h)

set(INCLUDES src ${WIRINGPI_INCLUDE_DIR})

# Get our NAP modules dependencies from module.json
module_json_to_cmake()

add_library(${PROJECT_NAME} SHARED ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)

# Remove lib prefix on Unix libraries
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDES})

target_link_libraries(${PROJECT_NAME} napcore ${DEPENDENT_NAP_MODULES} debug ${WIRINGPI_LIBRARIES_DEBUG} optimized ${WIRINGPI_LIBRARIES_RELEASE})

target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)

# Deploy module.json as MODULENAME.json alongside module post-build
copy_module_json_to_bin()

# Package into platform release
if(UNIX)
    set(LINUX_EXTRA_RPATH ../../../../thirdparty/wiringpi/lib)
endif()

package_module()

# Package rtmidi into platform release
install(FILES ${WIRINGPI_DIST_FILES} DESTINATION thirdparty/wiringpi)
install(DIRECTORY ${WIRINGPI_INCLUDE_DIR}/ DESTINATION thirdparty/wiringpi/include)

if(UNIX)
    file(GLOB WIRINGPI_DYLIBS ${WIRINGPI_LIBRARY_DIR}/libwiringpi${CMAKE_SHARED_LIBRARY_SUFFIX}*)
    install(FILES ${WIRINGPI_DYLIBS}
            DESTINATION thirdparty/wiringpi/lib)
endif()
