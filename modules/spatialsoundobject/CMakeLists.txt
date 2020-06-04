project(mod_spatialsoundobject)

file(GLOB_RECURSE sources src/*.cpp src/*.h src/*.hpp)

# Build source groups for input files maintaining their folder structure
# INPUT_FILES: Files to be added to source groups
# RELATIVE_TO_PATH: The root path against which our relative source group paths will be built
# GROUP_NAME_PREFIX: Any prefix that should be added onto the front of the group name
function(create_hierarchical_source_groups_for_files INPUT_FILES RELATIVE_TO_PATH GROUP_NAME_PREFIX)
    foreach(input_file ${INPUT_FILES})
        # Get the file's directory
        get_filename_component(THIS_FILE_DIR ${input_file} DIRECTORY)

        # Determine the file's directory path relative to the root
        file(RELATIVE_PATH RELATIVE_FILE_DIR ${RELATIVE_TO_PATH} ${THIS_FILE_DIR})

        # If it's in a subfolder..
        if(NOT ${RELATIVE_FILE_DIR} STREQUAL "")
            # Switch path separators
            string(REPLACE "/" "\\" RELATIVE_FILE_DIR ${RELATIVE_FILE_DIR})
            # Prepend path separator
            set(RELATIVE_FILE_DIR "\\${RELATIVE_FILE_DIR}")
        endif()

        # Add to source group
        source_group(${GROUP_NAME_PREFIX}${RELATIVE_FILE_DIR} FILES ${input_file})
    endforeach()
endfunction()

# Add source
file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE HEADERS src/*.h src/*.hpp)

# Create IDE groups
create_hierarchical_source_groups_for_files("${SOURCES}" ${CMAKE_CURRENT_SOURCE_DIR}/src "Sources")
create_hierarchical_source_groups_for_files("${HEADERS}" ${CMAKE_CURRENT_SOURCE_DIR}/src "Headers")


if (WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX2")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx")
endif()

# Get our NAP modules dependencies from module.json
module_json_to_cmake()

add_library(${PROJECT_NAME} SHARED ${sources})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)

target_include_directories(${PROJECT_NAME} PUBLIC src)

target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY _USE_MATH_DEFINES)

# link with external libs
if(NOT WIN32)
	target_compile_definitions(${PROJECT_NAME} PUBLIC HAVE_CONFIG_H)
endif()
target_link_libraries(${PROJECT_NAME} napcore ${DEPENDENT_NAP_MODULES})

# Deploy module.json as MODULENAME.json alongside module post-build
copy_module_json_to_bin()

# Package into platform release
if(APPLE)
    # A temporary ugly fix for inter-dependent modules and their RPATHs on macOS.  NAP-225.
    set(MACOS_EXTRA_RPATH_RELEASE ../../../../thirdparty/FreeImage/lib)
    set(MACOS_EXTRA_RPATH_DEBUG ${MACOS_EXTRA_RPATH_RELEASE})
endif()
package_module()

if (WIN32)
    # Install for fbxconverter
    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION tools/platform
    		CONFIGURATIONS Release)
endif()
