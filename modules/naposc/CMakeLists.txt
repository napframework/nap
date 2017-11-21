project(mod_naposc)

# add all cpp files to SOURCES
file(GLOB_RECURSE SOURCES src/ *.cpp *.h)

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
target_link_libraries(${PROJECT_NAME} napcore mod_napmath debug ${OSCPACK_LIBS_DEBUG} optimized ${OSCPACK_LIBS_RELEASE})

# Package into release build
if (WIN32)
	install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION "modules/${PROJECT_NAME}/lib/$<CONFIG>")
else()
	install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION "modules/${PROJECT_NAME}/lib/$<CONFIG>")
endif()

install(DIRECTORY "src/" DESTINATION "modules/${PROJECT_NAME}/include"
        FILES_MATCHING PATTERN "*.h")
