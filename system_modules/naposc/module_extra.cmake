if(NOT TARGET oscpack)
    find_package(oscpack REQUIRED)
endif()

if(NAP_BUILD_CONTEXT MATCHES "source")
    target_include_directories(${PROJECT_NAME} PUBLIC ${OSCPACK_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} debug ${OSCPACK_LIBS_DEBUG} optimized ${OSCPACK_LIBS_RELEASE})

    # Package oscpack into platform release
    install(FILES ${THIRDPARTY_DIR}/oscpack/source/LICENSE DESTINATION thirdparty/oscpack)
    install(FILES ${THIRDPARTY_DIR}/oscpack/source/README DESTINATION thirdparty/oscpack)
    install(DIRECTORY ${OSCPACK_INCLUDE_DIRS}/ DESTINATION thirdparty/oscpack/include PATTERN "*.cpp" EXCLUDE)
    install(DIRECTORY ${OSCPACK_LIBS_DIR}/ DESTINATION thirdparty/oscpack/lib)
else()
    set(MODULE_NAME_EXTRA_LIBS oscpack)

    add_include_to_interface_target(mod_naposc ${OSCPACK_INCLUDE_DIRS})

    # Install oscpack licenses into packaged project
    install(FILES ${THIRDPARTY_DIR}/oscpack/LICENSE DESTINATION licenses/oscpack)

    # Install oscpack shared lib into packaged project for Unix
    if(APPLE)
        install(FILES $<TARGET_FILE:oscpack> DESTINATION lib)    
    elseif(UNIX)
        file(GLOB OSCPACK_DYLIBS ${THIRDPARTY_DIR}/oscpack/lib/liboscpack*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
        install(FILES ${OSCPACK_DYLIBS} DESTINATION lib)
    endif()
endif()
