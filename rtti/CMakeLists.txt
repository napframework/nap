include(${NAP_ROOT}/cmake/packaging_macros.cmake)
project(naprtti)

file(GLOB_RECURSE SOURCES src/*.cpp src/*.h)
filter_platform_specific_files(SOURCES)

find_rttr()

find_package(rapidjson REQUIRED)
add_definitions(-DRAPIDJSON_HAS_STDSTRING=1)

# Let find_python find our prepackaged Python in thirdparty
find_python_in_thirdparty()
set(pybind11_DIR "${THIRDPARTY_DIR}/pybind11/install/share/cmake/pybind11")
find_package(pybind11 REQUIRED)

add_platform_specific_files("${WIN32_SOURCES}" "${MACOS_SOURCES}" "${LINUX_SOURCES}")
add_library(${PROJECT_NAME} SHARED ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Core)

if (MSVC)
    include(GenerateExportheader)
    GENERATE_EXPORT_HEADER(${PROJECT_NAME})
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME} NAP_SHARED_LIBRARY)
target_include_directories(${PROJECT_NAME} PUBLIC src)
target_include_directories(${PROJECT_NAME} PUBLIC ${RAPIDJSON_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PUBLIC ${pybind11_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} pybind11::embed)
target_link_libraries(${PROJECT_NAME} RTTR::Core)
target_link_libraries(${PROJECT_NAME} naputility)

if(APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework CoreServices")
elseif(UNIX)
    target_link_libraries(${PROJECT_NAME} dl pthread)
endif ()

if(APPLE)
    add_custom_command(TARGET ${PROJECT_NAME}
                       PRE_BUILD
                       COMMAND ${CMAKE_INSTALL_NAME_TOOL}
                               -id 
                               @rpath/librttr_core.0.9.6.dylib 
                               ${THIRDPARTY_DIR}/rttr/xcode/install/bin/librttr_core.0.9.6.dylib
                       COMMENT "Setting install name for rttr_core")

    add_custom_command(TARGET ${PROJECT_NAME}
                       PRE_BUILD
                       COMMAND ${CMAKE_INSTALL_NAME_TOOL}
                               -id 
                               @rpath/librttr_core_d.0.9.6.dylib 
                               ${THIRDPARTY_DIR}/rttr/xcode/install/bin/librttr_core_d.0.9.6.dylib
                       COMMENT "Setting install name for rttr_core_d")    
endif()

# Package into release build
if (WIN32)
    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION lib/$<CONFIG>
                                    ARCHIVE DESTINATION lib/$<CONFIG>)

    # Install for fbxconverter
    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION tools/platform
            CONFIGURATIONS Release)
elseif(APPLE)
    install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib/$<CONFIG>)  
else()
    install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib/${CMAKE_BUILD_TYPE})   
endif()

install(DIRECTORY "src/rtti/" DESTINATION "include/rtti/"
        FILES_MATCHING PATTERN "*.h")

# Package dependent lib into platform release --

# Package pybind
install(DIRECTORY ${THIRDPARTY_DIR}/pybind11/install/ DESTINATION thirdparty/pybind11)
install(FILES ${THIRDPARTY_DIR}/pybind11/LICENSE DESTINATION thirdparty/pybind11)

# Package RTTR
if (MSVC)
    install(DIRECTORY "${THIRDPARTY_DIR}/rttr/msvc64/install/" DESTINATION "thirdparty/rttr")
elseif(APPLE)
    set(BUILT_RPATH "@loader_path/../../thirdparty/rttr/bin/")
    set(BUILT_RPATH "${BUILT_RPATH};@loader_path/../../thirdparty/python/")
    set_target_properties(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "${BUILT_RPATH}")
    install(DIRECTORY "${THIRDPARTY_DIR}/rttr/xcode/install/" DESTINATION "thirdparty/rttr")
elseif(UNIX)
    install(DIRECTORY "${THIRDPARTY_DIR}/rttr/linux/install/" DESTINATION "thirdparty/rttr")
endif()
