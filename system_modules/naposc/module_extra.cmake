if(NOT TARGET oscpack)
    find_package(oscpack REQUIRED)
endif()

if(NAP_BUILD_CONTEXT MATCHES "source")
    target_include_directories(${PROJECT_NAME} PUBLIC ${OSCPACK_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} debug ${OSCPACK_LIBS_DEBUG} optimized ${OSCPACK_LIBS_RELEASE})

    # Package oscpack into platform release
    set(dest_dir system_modules/${PROJECT_NAME}/thirdparty/oscpack)
    install(FILES ${OSCPACK_DIR}/source/LICENSE
                  ${OSCPACK_DIR}/source/README
            DESTINATION ${dest_dir}/source
            )
    install(DIRECTORY ${OSCPACK_INCLUDE_DIRS}/ DESTINATION ${dest_dir}/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH}/include/oscpack PATTERN "*.cpp" EXCLUDE)
    install(DIRECTORY ${OSCPACK_LIBS_DIR}/ DESTINATION ${dest_dir}/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH}/lib)
else()
    set(MODULE_EXTRA_LIBS_OPTIMIZED ${OSCPACK_LIBS_RELEASE})
    set(MODULE_EXTRA_LIBS_DEBUG ${OSCPACK_LIBS_DEBUG})

    add_include_to_interface_target(naposc ${OSCPACK_INCLUDE_DIRS})

    # Install oscpack licenses into packaged project
    install(FILES ${OSCPACK_LICENSE_FILES} DESTINATION licenses/oscpack)

    # Install oscpack shared lib into packaged project for Unix
    if(APPLE)
        install(FILES ${OSCPACK_LIBS_RELEASE} DESTINATION lib)
    elseif(UNIX)
        file(GLOB OSCPACK_DYLIBS ${OSCPACK_LIBS_DIR}/liboscpack*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
        install(FILES ${OSCPACK_DYLIBS} DESTINATION lib)
    endif()
endif()
