find_path(
        ASSIMP_DIR
        NAMES include/assimp/scene.h
        HINTS
        ${CMAKE_CURRENT_LIST_DIR}/../assimp
        ${CMAKE_CURRENT_LIST_DIR}/../thirdparty/assimp/msvc
)

if (MSVC)
	set(ASSIMP_INCLUDE_DIRS ${ASSIMP_DIR}/msvc/include)
	set(ASSIMP_LIBRARIES ${ASSIMP_DIR}/msvc/lib64/assimp.lib)
elseif (APPLE)
	set(ASSIMP_INCLUDE_DIRS ${ASSIMP_DIR}/osx/include)
	set(ASSIMP_LIBRARIES ${ASSIMP_DIR}/osx/lib/assimp.a)
endif()

mark_as_advanced(GLM_INCLUDE_DIRS)