project(mod_napimgui)

# add all cpp files to SOURCES
file(GLOB_RECURSE SOURCES src/ src/imgui *.cpp *.h)

# LIBRARY

# compile shared lib as target
add_library(${PROJECT_NAME} SHARED ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)

# add include dirs
target_include_directories(${PROJECT_NAME} PUBLIC src)

# preprocessor
target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)

# link libs
target_link_libraries(${PROJECT_NAME} mod_naprender mod_napinput)
