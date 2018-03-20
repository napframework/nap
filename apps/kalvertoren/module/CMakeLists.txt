project(mod_napkalvertoren)

# add all cpp files to SOURCES
file(GLOB SOURCES src/*.cpp)
file(GLOB HEADERS src/*.h)
source_group("headers" FILES ${HEADERS})
source_group("sources" FILES ${SOURCES})

# LIBRARY

# compile shared lib as target
add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Apps)

# add include dirs
target_include_directories(${PROJECT_NAME} PUBLIC src)

# link with external libs
target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY _USE_MATH_DEFINES)
target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME})
set(DEPENDENT_NAP_MODULES mod_naprender nrender mod_napmath mod_napsdlinput mod_napsdlwindow mod_napcameracontrol mod_napapp mod_napartnet mod_napimgui)
target_link_libraries(${PROJECT_NAME} napcore ${DEPENDENT_NAP_MODULES})
