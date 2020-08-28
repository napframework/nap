# Exclude for Android
if(ANDROID)
    return()
endif()

project(mod_heightmap)

# add all cpp files to SOURCES
file(GLOB SOURCES src/*.cpp)
file(GLOB HEADERS src/*.h)
source_group("headers" FILES ${HEADERS})
source_group("sources" FILES ${SOURCES})

# Get our NAP modules dependencies from module.json
module_json_to_cmake()

# LIBRARY

# compile shared lib as target
add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Demos)
# Remove lib prefix on Unix libraries
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

# add include dirs
target_include_directories(${PROJECT_NAME} PUBLIC src)

# link with external libs
target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY _USE_MATH_DEFINES)
target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} napcore ${DEPENDENT_NAP_MODULES} nrender)

# Only include project modules when desired, only build when not packaging
exclude_from_build_when_packaging(FALSE)

# Deploy module.json as MODULENAME.json alongside module post-build
copy_module_json_to_bin()
