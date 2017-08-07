project(fbxconverter)

file(GLOB sources src/*.cpp src/*.h)
include_directories(src)

# Add TCLAP
find_package(tclap REQUIRED)
include_directories(${TCLAP_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} ${sources})
set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)")
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Tools)
target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME})

if (MSVC)
	# Link against setargv.obj so that the shell does wildcard expansion for us
	# See https://msdn.microsoft.com/en-us/library/8bch7bkk.aspx
	set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "setargv.obj")
endif()

target_link_libraries(${PROJECT_NAME} napcore mod_naprender nrender mod_napmath)

