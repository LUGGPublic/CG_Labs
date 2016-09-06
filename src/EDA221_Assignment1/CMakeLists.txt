cmake_minimum_required (VERSION 3.0)

project (EDA221_Assignment1 LANGUAGES CXX)

set (
	SOURCES

	"main.cpp"
	"SolarSystem.cpp"
	"SolarSystem.h"
)

source_group (
	EDA221${PATH_SEP}Assignment1

	FILES
	${PROJECT_SOURCE_DIR}/main.cpp
	${PROJECT_SOURCE_DIR}/SolarSystem.cpp
	${PROJECT_SOURCE_DIR}/SolarSystem.hpp
	${SHADERS_DIR}/EDA221_Assignment1/LambertTexture.vert
	${SHADERS_DIR}/EDA221_Assignment1/LambertTexture.frag
)

add_executable (${PROJECT_NAME} ${SOURCES})

target_include_directories (${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/src/external")
target_include_directories (${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_include_directories (${PROJECT_NAME} PRIVATE "${CMAKE_BINARY_DIR}")
target_include_directories (${PROJECT_NAME} PRIVATE ${IMGUI_INCLUDE_DIRS})
target_include_directories (${PROJECT_NAME} PRIVATE ${GLM_INCLUDE_DIRS})
target_include_directories (${PROJECT_NAME} PRIVATE ${ASSIMP_INCLUDE_DIRS})

if (NOT (CMAKE_CXX_COMPILER_ID MATCHES "MSVC"))
	target_compile_options (${PROJECT_NAME} PRIVATE "-std=c++14")
endif ()
target_compile_options (${PROJECT_NAME} PRIVATE "-Wall")

add_dependencies (${PROJECT_NAME} bonobo)

target_link_libraries (${PROJECT_NAME} ${OPENGL_gl_LIBRARY} ${IMGUI_LIBRARY} ${ASSIMP_LIBRARIES})
target_link_libraries (${PROJECT_NAME} bonobo)
if (WIN32)
	set_property( DIRECTORY "${PROJECT_SOURCE_DIR}" PROPERTY VS_STARTUP_PROJECT "${PROJECT_NAME}" )
	file (MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/Debug)
	file (MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/Release)
	add_custom_command (TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/assimp/bin/assimp-vc140-mt.dll ${CMAKE_BINARY_DIR}/src/EDA221_Assignment1/Debug)
	add_custom_command (TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/assimp/bin/assimp-vc140-mt.dll ${CMAKE_BINARY_DIR}/src/EDA221_Assignment1/Release)
endif ()


if (APPLE)
	find_library (COCOA_LIBRARY Cocoa)
	find_library (IOKIT_LIBRARY IOKit)
	find_library (CORE_VIDEO_LIBRARY CoreVideo)
	find_library (CORE_FOUNDATION_LIBRARY CoreFoundation)
	set (EXTRA_LIBS ${COCOA_LIBRARY} ${IOKIT_LIBRARY} ${CORE_VIDEO_LIBRARY} ${CORE_FOUNDATION_LIBRARY})
elseif (UNIX)
	set (EXTRA_LIBS dl)
endif ()

target_link_libraries (${PROJECT_NAME} ${EXTRA_LIBS})
