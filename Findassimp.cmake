


if (MSVC)
	find_path(
	        ASSIMP_DIR
	        NAMES include/assimp/scene.h
	        HINTS
	        ${CMAKE_CURRENT_LIST_DIR}/../thirdparty/assimp/msvc
	)
	set(ASSIMP_INCLUDE_DIRS ${ASSIMP_DIR}/include)
	set(ASSIMP_LIBRARIES ${ASSIMP_DIR}/lib64/assimp.lib)
elseif (APPLE)
	find_path(
	        ASSIMP_DIR
	        NAMES include/assimp/scene.h
	        HINTS
	        ${CMAKE_CURRENT_LIST_DIR}/../thirdparty/assimp/osx
	)
	set(ASSIMP_INCLUDE_DIRS ${ASSIMP_DIR}/include)
	set(ASSIMP_LIBRARIES ${ASSIMP_DIR}/lib/assimp.a)
endif()

mark_as_advanced(GLM_INCLUDE_DIRS)