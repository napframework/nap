project(mod_naposc)

# add all cpp files to SOURCES
file(GLOB_RECURSE SOURCES src/*.cpp src/*.h)

find_package(oscpack REQUIRED)

# LIBRARY

# compile shared lib as target
add_library(${PROJECT_NAME} SHARED ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)

# add include dirs
target_include_directories(${PROJECT_NAME} PUBLIC src ${OSCPACK_INCLUDE_DIRS})

# preprocessor
target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)

# link libs
set(DEPENDENT_NAP_MODULES mod_napmath mod_napscene )
target_link_libraries(${PROJECT_NAME} napcore ${DEPENDENT_NAP_MODULES} debug ${OSCPACK_LIBS_DEBUG} optimized ${OSCPACK_LIBS_RELEASE})

# Package into platform release
package_module()

# Package oscpack into platform release
install(FILES ${THIRDPARTY_DIR}/oscpack/LICENSE DESTINATION thirdparty/oscpack)
install(FILES ${THIRDPARTY_DIR}/oscpack/README DESTINATION thirdparty/oscpack)
install(DIRECTORY ${THIRDPARTY_DIR}/oscpack/osc DESTINATION thirdparty/oscpack/include
        PATTERN "*.cpp" EXCLUDE)
if(MSVC)
    install(DIRECTORY ${THIRDPARTY_DIR}/oscpack/msvc64/ DESTINATION thirdparty/oscpack/lib)
elseif(APPLE)
    install(DIRECTORY ${THIRDPARTY_DIR}/oscpack/xcode/ DESTINATION thirdparty/oscpack/lib)
else()
    install(DIRECTORY ${THIRDPARTY_DIR}/oscpack/linux/ DESTINATION thirdparty/oscpack/lib)
endif()