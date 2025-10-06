project(naprtti)

file(GLOB_RECURSE SOURCES src/*.cpp src/*.h)
filter_platform_specific_files(SOURCES)

find_rttr()

set(rapidjson_FIND_QUIETLY TRUE)
find_package(rapidjson REQUIRED)
add_definitions(-DRAPIDJSON_HAS_STDSTRING=1)

add_platform_specific_files("${WIN32_SOURCES}" "${LINUX_SOURCES}")
add_library(${PROJECT_NAME} SHARED ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Core)

# Remove lib prefix on Unix libraries
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

if (MSVC)
    include(GenerateExportheader)
    GENERATE_EXPORT_HEADER(${PROJECT_NAME})
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME} NAP_SHARED_LIBRARY)
target_include_directories(${PROJECT_NAME} PUBLIC src)
target_include_directories(${PROJECT_NAME} PUBLIC ${RAPIDJSON_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} naputility)
target_link_libraries(${PROJECT_NAME} RTTR::Core)

if(UNIX)
    target_link_libraries(${PROJECT_NAME} dl pthread)
endif ()

# Windows thirdparty DLL copying
if(WIN32)
    # RTTR
    copy_files_to_bin($<TARGET_FILE:RTTR::Core>)
endif()

# Package into release build
if (WIN32)
    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION lib/$<CONFIG>-${ARCH}
                                    ARCHIVE DESTINATION lib/$<CONFIG>-${ARCH})

    # Install for fbxconverter
    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION tools/buildsystem
            CONFIGURATIONS Release)

    if(PACKAGE_PDBS)
        install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION lib/$<CONFIG>-${ARCH})
    endif()
elseif(UNIX)
    install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib/${CMAKE_BUILD_TYPE}-${ARCH})
endif()

# RTTI headers
install(DIRECTORY src/rtti/ DESTINATION include/rtti/
        FILES_MATCHING PATTERN "*.h")


# Package dependent lib into framework release --

# Package RTTR
if (MSVC)
    install(DIRECTORY ${THIRDPARTY_DIR}/rttr/msvc/x86_64 DESTINATION thirdparty/rttr/msvc)
elseif(UNIX)
    install(DIRECTORY ${THIRDPARTY_DIR}/rttr/linux/${ARCH} DESTINATION thirdparty/rttr/linux)
endif()
install(FILES ${THIRDPARTY_DIR}/rttr/source/LICENSE.txt DESTINATION thirdparty/rttr/source)

# Package RapidJSON
install(FILES ${THIRDPARTY_DIR}/rapidjson/license.txt
              ${THIRDPARTY_DIR}/rapidjson/readme.md
        DESTINATION thirdparty/rapidjson)
install(DIRECTORY ${THIRDPARTY_DIR}/rapidjson/include DESTINATION thirdparty/rapidjson)
