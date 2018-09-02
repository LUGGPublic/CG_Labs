cmake_minimum_required (VERSION 3.0)

set (
	ASSIGNMENT2_SOURCES

	"assignment2.cpp"
	"assignment2.hpp"
)

source_group (
	EDAN35${PATH_SEP}Assignment2

	FILES
	${PROJECT_SOURCE_DIR}/assignment2.cpp
	${PROJECT_SOURCE_DIR}/assignment2.hpp
)

luggcgl_new_assignment ("EDAN35_Assignment2" "${ASSIGNMENT2_SOURCES}" "")

if (WIN32)
	string (REGEX REPLACE "v([0-9][0-9])([0-9])" "vc\\1\\2" VS_TOOLSET_VERSION "${CMAKE_VS_PLATFORM_TOOLSET}" )
	set (assimp_dll "${FETCHCONTENT_BASE_DIR}/assimp-install/bin/assimp-${VS_TOOLSET_VERSION}-mt.dll")
	set (target_dir ${CMAKE_BINARY_DIR}/src/EDAN35)

	add_custom_command (
		TARGET EDAN35_Assignment2
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy ${assimp_dll} ${target_dir}/$<CONFIG>
		COMMENT "Copying dependent DLLs"
	)
endif ()
