macro(dist_project_template)
    set(NAP_ROOT "${CMAKE_SOURCE_DIR}/../../")
    include(${NAP_ROOT}/cmake/targetarch.cmake)

    # Set our default build type if we haven't specified one (Linux)
	set_default_build_type()

    set(CMAKE_CXX_STANDARD 14)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
    if (WIN32)
        if(MSVC)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4244 /wd4305 /wd4996 /wd4267 /wd4018 /wd4251 /MP /bigobj")
            set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
            set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
    		set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
        else()
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse -Wa,-mbig-obj")
    #        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse")
        endif()
    elseif(UNIX)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wno-format-security -Wno-switch")
    endif()

    if(APPLE)
        set(CMAKE_OSX_DEPLOYMENT_TARGET 10.9)
    endif()

    cmake_policy(SET CMP0020 NEW)
    cmake_policy(SET CMP0043 NEW)

    # Allow extra Find{project}.cmake files to be found by projects
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
    list(APPEND CMAKE_MODULE_PATH "${NAP_ROOT}/cmake")

    include(${NAP_ROOT}/cmake/configure.cmake)

    if(WIN32)
        if(MINGW)
            # Copy required MinGW dll's to bin dir
            get_filename_component(COMPILER_DIR ${CMAKE_CXX_COMPILER} PATH)
            set(MODULES gcc_s_dw2-1 stdc++-6 winpthread-1)
            foreach (MOD ${MODULES})
                find_library(_LIB NAMES ${MOD} HINTS ${COMPILER_DIR})
                message(STATUS "Copy ${_LIB} to ${BIN_DIR}")
                file(COPY ${_LIB} DESTINATION ${BIN_DIR})
                unset(_LIB CACHE)
            endforeach ()
        endif()
    endif()

    if (APPLE)
        set(GENERATE_XCODE_PROJECT_TARGET generateXcodeProject)
        add_custom_target(generateXcodeProject cmake -H. -Bxcode -G Xcode -DCMAKE_BUILD_TYPE=Debug ../)
    endif()

    include_directories(${NAP_ROOT}/include/)

    #add all cpp files to SOURCES
    file(GLOB SOURCES src/*.cpp)
    file(GLOB HEADERS src/*.h)
    file(GLOB SHADERS shaders/*.frag shaders/*.vert)
    file(GLOB DATA data/*)

    # Create IDE groups
    source_group("headers" FILES ${HEADERS})
    source_group("sources" FILES ${SOURCES})
    source_group("shaders" FILES ${SHADERS})

    add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${SHADERS})
    if (WIN32)
        set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)")
        if (${CMAKE_VERSION} VERSION_GREATER "3.6.0") 
            # Set project as startup project in Visual Studio
            set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
        endif()
    endif()
    target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME})

    # Pull in NAP core
    find_package(napcore REQUIRED)
    find_package(naprtti REQUIRED)

    # Find each NAP module
    foreach(NAP_MODULE ${NAP_MODULES})
        find_nap_module(${NAP_MODULE})
    endforeach()

    target_link_libraries(${PROJECT_NAME} napcore naprtti RTTR::Core ${NAP_MODULES} ${PYTHON_LIBRARIES} ${SDL2_LIBRARY})

    # Copy data to bin post-build
    copy_dir_to_bin(${CMAKE_CURRENT_LIST_DIR}/data data)
    copy_dir_to_bin(${CMAKE_CURRENT_LIST_DIR}/shaders shaders)
    dist_export_fbx(${CMAKE_CURRENT_LIST_DIR}/data/${PROJECT_NAME})
endmacro()

# Set a default build type if none was specified (single-configuration generators only, ie. Linux)
macro(set_default_build_type)
	if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	    message(STATUS "Setting build type to 'Debug' as none was specified.")
	    set(CMAKE_BUILD_TYPE "Debug")
	endif()
endmacro()


# Somewhat temporary generic way to import each module for different configurations.  I think we'll need make proper package files for
# each module in the long run
# TODO let's avoid per-module cmake package files for now.. but probably need to re-address later
macro(find_nap_module MODULE_NAME)
    add_library(${MODULE_NAME} SHARED IMPORTED)

    message("Adding lib path for ${MODULE_NAME}")
    if (WIN32)
        set(MOD_RELEASE_DLL ${NAP_ROOT}/modules/${MODULE_NAME}/lib/Release/${MODULE_NAME}.dll)
        set(MOD_DEBUG_DLL ${NAP_ROOT}/modules/${MODULE_NAME}/lib/Debug/${MODULE_NAME}.dll)
        set(MOD_IMPLIB_DEBUG ${NAP_ROOT}/modules/${MODULE_NAME}/lib/Debug/${MODULE_NAME}.lib)
        set(MOD_IMPLIB_RELEASE ${NAP_ROOT}/modules/${MODULE_NAME}/lib/Release/${MODULE_NAME}.lib)
    elseif (APPLE)
        set(MOD_RELEASE_DLL ${NAP_ROOT}/modules/${MODULE_NAME}/lib/Release/lib${MODULE_NAME}.dylib)
        set(MOD_DEBUG_DLL ${NAP_ROOT}/modules/${MODULE_NAME}/lib/Debug/lib${MODULE_NAME}.dylib)
    elseif (UNIX)
        set(MOD_RELEASE_DLL ${NAP_ROOT}/modules/${MODULE_NAME}/lib/Release/lib${MODULE_NAME}.so)
        set(MOD_DEBUG_DLL ${NAP_ROOT}/modules/${MODULE_NAME}/lib/Debug/lib${MODULE_NAME}.so)
    endif()

    set_target_properties(${MODULE_NAME} PROPERTIES
        IMPORTED_CONFIGURATIONS "Debug;Release;MinSizeRel;RelWithDebInfo"
        IMPORTED_LOCATION_RELEASE ${MOD_RELEASE_DLL}
        IMPORTED_LOCATION_DEBUG ${MOD_DEBUG_DLL}
        IMPORTED_LOCATION_MINSIZEREL ${MOD_RELEASE_DLL}
        IMPORTED_LOCATION_RELWITHDEBINFO ${MOD_RELEASE_DLL}
    )

    # Add module includes
    message("Adding include for ${NAP_MODULE}")
    target_include_directories(${PROJECT_NAME} PUBLIC ${NAP_ROOT}/modules/${NAP_MODULE}/include/)

    if (WIN32)
        # Set Windows .lib locations
        # TODO test test test
        set_target_properties(${MODULE_NAME} PROPERTIES
            IMPORTED_IMPLIB_RELEASE ${MOD_IMPLIB_RELEASE}
            IMPORTED_IMPLIB_DEBUG ${MOD_IMPLIB_DEBUG}
            IMPORTED_IMPLIB_MINSIZEREL ${MOD_IMPLIB_RELEASE}
            IMPORTED_IMPLIB_RELWITHDEBINFO ${MOD_IMPLIB_RELEASE}
        )

        # Copy over module DLLs post-build
        add_custom_command(
            TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${MODULE_NAME}> $<TARGET_FILE_DIR:${PROJECT_NAME}>/
        )
    endif()

    # Bring in any additional module requirements
    set(MODULE_EXTRA_CMAKE_PATH ${NAP_ROOT}/modules/${MODULE_NAME}/moduleExtra.cmake)
    if (EXISTS ${MODULE_EXTRA_CMAKE_PATH})
        include (${MODULE_EXTRA_CMAKE_PATH})
    endif()
endmacro()

macro(dist_export_fbx SRCDIR)
    set(TOOLS_DIR "${CMAKE_CURRENT_LIST_DIR}/../../tools/")

    # Set the binary name
    set(FBXCONVERTER_BIN "${TOOLS_DIR}/fbxconverter")

    # Set project data out path
    set(OUTDIR "$<TARGET_FILE_DIR:${PROJECT_NAME}>/data/${PROJECT_NAME}")

    # Ensure data output directory for project exists
    add_custom_command(TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTDIR}
        COMMENT "Ensure project output directory exists for fbxconverter")

    # Do the export
    add_custom_command(TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND "${FBXCONVERTER_BIN}" -o ${OUTDIR} "${SRCDIR}/*.fbx"
        COMMENT "Export FBX in '${SRCDIR}'")
endmacro()
