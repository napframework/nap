# Exclude for Android
if(ANDROID)
    return()
endif()

project(licensegenerator)

file(GLOB sources src/*.cpp src/*.h)
include_directories(src)

# Add TCLAP
set(TCLAP_FIND_QUIETLY TRUE)
find_package(tclap REQUIRED)
include_directories(${TCLAP_INCLUDE_DIRS})

# Add cryptopp
find_package(cryptopp REQUIRED)
include_directories(${CRYPTOPP_INCLUDE_DIRS} ${TCLAP_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} ${sources})
set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)")
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Tools)
target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} cryptopp)

#set(DEPENDENT_NAP_MODULES mod_naprender mod_napmath mod_napscene)
#target_link_libraries(${PROJECT_NAME} napcore ${DEPENDENT_NAP_MODULES})

# Add the runtime paths for RTTR on macOS
if(APPLE)
    add_macos_rttr_rpath()
endif()

if(WIN32)
  copy_cryptopp_dll()
endif()

# ==================================

# Package into NAP release
set(LICENSE_GENERATOR_PACKAGED_BUILD_TYPE Release)
set(LICENSE_GENERATOR_LOCATION tools/license)

install(TARGETS ${PROJECT_NAME} 
        DESTINATION ${LICENSE_GENERATOR_LOCATION}
        CONFIGURATIONS ${LICENSE_GENERATOR_PACKAGED_BUILD_TYPE})

if (WIN32)   
    # TODO Update to find the libraries then install locating them via TARGET_FILE
    install(FILES ${CRYPTOPP_DLL_RELEASE} DESTINATION ${LICENSE_GENERATOR_LOCATION})

    if(PACKAGE_PDBS)
        install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> 
                DESTINATION ${LICENSE_GENERATOR_LOCATION}
                CONFIGURATIONS ${LICENSE_GENERATOR_PACKAGED_BUILD_TYPE}
                )           
      endif()
  endif()