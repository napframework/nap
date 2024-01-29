project(naprtti)

file(GLOB_RECURSE SOURCES src/*.cpp src/*.h)
filter_platform_specific_files(SOURCES)

find_rttr()

set(rapidjson_FIND_QUIETLY TRUE)
find_package(rapidjson REQUIRED)
add_definitions(-DRAPIDJSON_HAS_STDSTRING=1)

add_platform_specific_files("${WIN32_SOURCES}" "${MACOS_SOURCES}" "${LINUX_SOURCES}")
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

if (NAP_ENABLE_PYTHON)
    # Find the Python interpreter in thirdparty
    configure_python()

    # Pybind uses the interpreter to locate the python libraries and includes, next to pybind includes
    set(pybind11_parent_dir ${NAP_ROOT}/system_modules/nappython/thirdparty/pybind11)
    set(pybind11_DIR ${pybind11_parent_dir}/install/share/cmake/pybind11)
    find_package(pybind11 REQUIRED)

    # Add pybind includes and libraries
    target_include_directories(${PROJECT_NAME} PUBLIC ${pybind11_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} pybind11::embed)
    target_link_directories(${PROJECT_NAME} PUBLIC ${PYTHON_LIB_DIR})
endif()

target_link_libraries(${PROJECT_NAME} naputility)
target_link_libraries(${PROJECT_NAME} RTTR::Core)

if(APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework CoreServices")
elseif(UNIX)
    target_link_libraries(${PROJECT_NAME} dl pthread)
endif ()

# Windows thirdparty DLL copying
if(WIN32)
    # RTTR
    copy_files_to_bin($<TARGET_FILE:RTTR::Core>)

    # Python
    if(NAP_ENABLE_PYTHON)
        file(GLOB PYTHON_DLLS ${THIRDPARTY_DIR}/python/msvc/x86_64/*.dll)
        copy_files_to_bin(${PYTHON_DLLS})
    endif()
endif()

if(APPLE)
    add_custom_command(TARGET ${PROJECT_NAME}
                       PRE_BUILD
                       COMMAND ${CMAKE_INSTALL_NAME_TOOL}
                               -id
                               @rpath/librttr_core.0.9.6.dylib
                               ${THIRDPARTY_DIR}/rttr/macos/${ARCH}/bin/librttr_core.0.9.6.dylib
                       COMMENT "Setting install name for rttr_core")

    add_custom_command(TARGET ${PROJECT_NAME}
                       PRE_BUILD
                       COMMAND ${CMAKE_INSTALL_NAME_TOOL}
                               -id
                               @rpath/librttr_core_d.0.9.6.dylib
                               ${THIRDPARTY_DIR}/rttr/macos/${ARCH}/bin/librttr_core_d.0.9.6.dylib
                       COMMENT "Setting install name for rttr_core_d")
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
elseif(APPLE)
    install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib/$<CONFIG>-${ARCH})
else()
    install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib/${CMAKE_BUILD_TYPE}-${ARCH})
endif()

# RTTI headers
install(DIRECTORY src/rtti/ DESTINATION include/rtti/
        FILES_MATCHING PATTERN "*.h")


# Package dependent lib into framework release --

# Package pybind11
if (NAP_ENABLE_PYTHON)
    install(DIRECTORY ${pybind11_parent_dir}/install DESTINATION system_modules/nappython/thirdparty/pybind11)
    install(FILES ${pybind11_parent_dir}/LICENSE DESTINATION system_modules/nappython/thirdparty/pybind11)
endif()

# Package RTTR
if (MSVC)
    install(DIRECTORY ${THIRDPARTY_DIR}/rttr/msvc/x86_64 DESTINATION thirdparty/rttr/msvc)
elseif(APPLE)
    set(BUILT_RPATH "@loader_path/../../thirdparty/rttr/bin/")
    set(BUILT_RPATH "${BUILT_RPATH};@loader_path/../../thirdparty/python/")
    set_target_properties(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "${BUILT_RPATH}")
    install(DIRECTORY ${THIRDPARTY_DIR}/rttr/macos/x86_64 DESTINATION thirdparty/rttr/macos)
elseif(UNIX)
    install(DIRECTORY ${THIRDPARTY_DIR}/rttr/linux/${ARCH} DESTINATION thirdparty/rttr/linux)
endif()
install(FILES ${THIRDPARTY_DIR}/rttr/source/LICENSE.txt DESTINATION thirdparty/rttr/source)

# Package RapidJSON
install(FILES ${THIRDPARTY_DIR}/rapidjson/license.txt
              ${THIRDPARTY_DIR}/rapidjson/readme.md
        DESTINATION thirdparty/rapidjson)
install(DIRECTORY ${THIRDPARTY_DIR}/rapidjson/include DESTINATION thirdparty/rapidjson)
