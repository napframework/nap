project(napcore)

file(GLOB_RECURSE SOURCES src/ *.cpp *.h)
file(GLOB_RECURSE HEARERS src/ *.h)
filter_platform_specific_files(SOURCES)

add_platform_specific_files("${WIN32_SOURCES}" "${OSX_SOURCES}" "${LINUX_SOURCES}")

add_library(${PROJECT_NAME} SHARED ${SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Core)
target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)
if (MSVC)
    include(GenerateExportheader)
    GENERATE_EXPORT_HEADER(${PROJECT_NAME})
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME})
if(MSVC)
	target_compile_definitions(${PROJECT_NAME} PUBLIC NOMINMAX)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC src)
target_link_libraries(${PROJECT_NAME} naputility naprtti)

if(APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework CoreServices")
elseif(UNIX)
    target_link_libraries(${PROJECT_NAME} dl pthread)
endif ()

set_property(TARGET ${PROJECT_NAME} PROPERTY PUBLIC_HEADER ${HEADERS})

nap_install_lib(${PROJECT_NAME})
