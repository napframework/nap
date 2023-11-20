project(keygen)

file(GLOB sources src/*.cpp src/*.h)
include_directories(src)

# Add TCLAP
set(TCLAP_FIND_QUIETLY TRUE)
find_package(tclap REQUIRED)
include_directories(${TCLAP_INCLUDE_DIRS})

# Add cryptopp
list(APPEND CMAKE_MODULE_PATH ${NAP_ROOT}/system_modules/naplicense/thirdparty/cmake_find_modules)
find_package(cryptopp REQUIRED)
include_directories(${CRYPTOPP_INCLUDE_DIRS} ${TCLAP_INCLUDE_DIRS})

# Add openssl
list(APPEND CMAKE_MODULE_PATH ${NAP_ROOT}/system_modules/napopenssl/thirdparty/cmake_find_modules)
find_package(openssl REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} ${sources})
set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)")
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Tools)
target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} cryptopp)
target_link_libraries(${PROJECT_NAME} ${LIBCRYPTO_LIB} ${LIBSSL_LIB})

#set(DEPENDENT_NAP_MODULES naprender napmath napscene)
#target_link_libraries(${PROJECT_NAME} napcore ${DEPENDENT_NAP_MODULES})

# Add the runtime paths for RTTR on macOS
if(APPLE)
    macos_add_rttr_rpath()
endif()

if(WIN32)
  copy_cryptopp_dll()
endif()

# ==================================

# Package into NAP release
set(KEYGEN_PACKAGED_BUILD_TYPE Release)
set(KEYGEN_INSTALL_LOCATION tools/license)

install(TARGETS ${PROJECT_NAME} 
        DESTINATION ${KEYGEN_INSTALL_LOCATION}
        CONFIGURATIONS ${KEYGEN_PACKAGED_BUILD_TYPE})

if (WIN32)   
    # TODO Update to find the libraries then install locating them via TARGET_FILE
    install(FILES ${CRYPTOPP_DLL_RELEASE} DESTINATION ${KEYGEN_INSTALL_LOCATION})

    if(PACKAGE_PDBS)
        install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> 
                DESTINATION ${KEYGEN_INSTALL_LOCATION}
                CONFIGURATIONS ${KEYGEN_PACKAGED_BUILD_TYPE}
                )           
      endif()
  endif()
