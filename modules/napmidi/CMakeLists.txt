project(mod_napmidi)

find_package(rtmidi REQUIRED)

file(GLOB_RECURSE sources src/*.cpp src/*.h)

set(INCLUDES src ${RTMIDI_INCLUDE_DIR})

add_library(${PROJECT_NAME} SHARED ${sources})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)

target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDES})

set(DEPENDENT_NAP_MODULES mod_napscene)
target_link_libraries(${PROJECT_NAME} napcore ${DEPENDENT_NAP_MODULES} debug ${RTMIDI_LIBRARIES_DEBUG} optimized ${RTMIDI_LIBRARIES_RELEASE} )

target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)

# Package into platform release
package_module()

# Set packaged Linux RPATH 
if (UNIX AND NOT APPLE)
    set_installed_module_rpath_for_dependent_modules("${DEPENDENT_NAP_MODULES}" ${PROJECT_NAME})
endif()