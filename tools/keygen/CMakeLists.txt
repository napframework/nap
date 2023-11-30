project(keygen)

file(GLOB sources src/*.cpp src/*.h)
include_directories(src)

# Add TCLAP
set(TCLAP_FIND_QUIETLY TRUE)
find_package(tclap REQUIRED)
include_directories(${TCLAP_INCLUDE_DIRS})

# Add openssl
list(APPEND CMAKE_MODULE_PATH ${NAP_ROOT}/system_modules/napopenssl/thirdparty/cmake_find_modules)
find_package(openssl REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIRS})

# Add napopenssl utility methods
include_directories("${NAP_ROOT}/system_modules/napopenssl/src")
file(GLOB NAP_OPENSSL_SOURCES "${NAP_ROOT}/system_modules/napopenssl/src/opensslapi.cpp" "${NAP_ROOT}/system_modules/napopenssl/src/opensslapi.h")

add_executable(${PROJECT_NAME} ${sources} ${NAP_OPENSSL_SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)")
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Tools)
target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} ${LIBCRYPTO_LIB} ${LIBSSL_LIB})

#set(DEPENDENT_NAP_MODULES naprender napmath napscene)
#target_link_libraries(${PROJECT_NAME} napcore ${DEPENDENT_NAP_MODULES})

# Add the runtime paths for RTTR on macOS
if(APPLE)
    macos_add_rttr_rpath()
endif()

# ==================================

# Package into NAP release
set(KEYGEN_PACKAGED_BUILD_TYPE Release)
set(KEYGEN_INSTALL_LOCATION tools/license)

install(TARGETS ${PROJECT_NAME} 
        DESTINATION ${KEYGEN_INSTALL_LOCATION}
        CONFIGURATIONS ${KEYGEN_PACKAGED_BUILD_TYPE})
