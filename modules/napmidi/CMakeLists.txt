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
if(UNIX)
    set(UNIX_EXTRA_RPATH "../../../../thirdparty/rtmidi/lib")
endif()
package_module()

# Package rtmidi into platform release
install(FILES ${THIRDPARTY_DIR}/rtmidi/linux/install/README.md DESTINATION thirdparty/rtmidi)
install(DIRECTORY ${THIRDPARTY_DIR}/rtmidi/linux/install/include/rtmidi/ DESTINATION thirdparty/rtmidi/include)
if(MSVC)
    file(GLOB RTMIDI_STATIC_LIBS ${THIRDPARTY_DIR}/rtmidi/bin/msvc/rtmidi*${CMAKE_STATIC_LIBRARY_SUFFIX})
    install(FILES ${RTMIDI_STATIC_LIBS}
            DESTINATION thirdparty/rtmidi/bin)
elseif(APPLE)
    install(FILES ${THIRDPARTY_DIR}/rtmidi/bin/osx/librtmidi.4.dylib
            DESTINATION thirdparty/rtmidi/lib)
elseif(UNIX)
    file(GLOB RTMIDI_DYLIBS ${RTMIDI_DIR}/linux/install/lib/librtmidi${CMAKE_SHARED_LIBRARY_SUFFIX}*)
    install(FILES ${RTMIDI_DYLIBS}
            DESTINATION thirdparty/rtmidi/lib)
endif()
