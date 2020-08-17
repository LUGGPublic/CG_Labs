# As we use the VERSION option---which was introduced in CMAKE 3.0---of the
# project() command, set the minimum required version to 3.0.
cmake_minimum_required (VERSION 3.0)

# Workaround for the lab computers
if (WIN32)
	set (LUGGCGL_TMP_DIR ${CMAKE_SOURCE_DIR}/../Temp)
	if (NOT EXISTS ${LUGGCGL_TMP_DIR})
		file (MAKE_DIRECTORY ${LUGGCGL_TMP_DIR})
	endif ()
	set (ENV{TMP} ${LUGGCGL_TMP_DIR})
endif ()

# Set the project name, versions and languages used.
project (
	CG_Labs
	VERSION 2019.1
	LANGUAGES CXX C
)


# Set build type
set (CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo")
if (NOT CMAKE_BUILD_TYPE)
	set (
		CMAKE_BUILD_TYPE
		"Release"
		CACHE STRING
		"Choose the type of build, options are: ${CMAKE_CONFIGURATION_TYPES}."
		FORCE
	)
endif (NOT CMAKE_BUILD_TYPE)
set_property (CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS ${CMAKE_CONFIGURATION_TYPES})


# To be able to dowload and compile the dependencies
find_package (Git REQUIRED)
set (FETCHCONTENT_BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dependencies")

if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/dependencies")
	file (MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/dependencies")
endif ()

# assimp is used for loading the objects’s models
set (LUGGCGL_ASSIMP_MIN_VERSION 5.0.0)
set (LUGGCGL_ASSIMP_DOWNLOAD_VERSION 5.0.1)
include (CMake/InstallAssimp.cmake)
find_package (assimp ${LUGGCGL_ASSIMP_MIN_VERSION} REQUIRED)
link_directories (${ASSIMP_LIBRARY_DIRS})

# GLFW is used for inputs and windows handling
set (LUGGCGL_GLFW_MIN_VERSION 3.2.0)
set (LUGGCGL_GLFW_DOWNLOAD_VERSION 3.2.1)
include (CMake/InstallGLFW.cmake)
find_package (glfw3 ${LUGGCGL_GLFW_MIN_VERSION} REQUIRED)

# GLM is used for matrices, vectors and camera handling
set (LUGGCGL_GLM_DOWNLOAD_VERSION 0.9.9.5)
include (CMake/InstallGLM.cmake)
find_package (glm ${LUGGCGL_GLM_DOWNLOAD_VERSION} EXACT REQUIRED)

# TinyFileDialogs is used for displaying error popups.
include (CMake/InstallTinyFileDialogs.cmake)

# stb is used for loading in image files.
include (CMake/InstallSTB.cmake)

# Resources are found in an external archive
include (CMake/RetrieveResourceArchive.cmake)


# Configure *C++ Environment Variables*
set (MSAA_RATE "1" CACHE STRING "Window MSAA rate")
set (WIDTH "1600" CACHE STRING "Window width")
set (HEIGHT "900" CACHE STRING "Window height")
set (ROOT_DIR "${PROJECT_SOURCE_DIR}")
configure_file ("${PROJECT_SOURCE_DIR}/src/core/config.hpp.in" "${PROJECT_BINARY_DIR}/config.hpp")


# Set up Doxygen documentation generation
option (LUGGCGL_BUILD_DOCUMENTATION "Build documentation for Lund University Computer Graphics Labs" OFF)
if (LUGGCGL_BUILD_DOCUMENTATION)
	find_package (Doxygen REQUIRED)
	set (LUGGCGL_VERSION_NUMBER "${CMAKE_PROJECT_VERSION}")
	set (LUGGCGL_DOC_INPUT "${CMAKE_SOURCE_DIR}/src")
	set (LUGGCGL_DOC_EXCLUDE ${CMAKE_SOURCE_DIR}/src/external)
	set (LUGGCGL_DOC_OUTPUT "${CMAKE_BINARY_DIR}/doc")
	if (NOT EXISTS "${LUGGCGL_DOC_OUTPUT}")
		file (MAKE_DIRECTORY "${LUGGCGL_DOC_OUTPUT}")
	endif ()
	configure_file ("${CMAKE_SOURCE_DIR}/Doxyfile.in" "${CMAKE_BINARY_DIR}/doc/Doxyfile" @ONLY)
	add_custom_target (
		doc
		COMMAND ${DOXYGEN_EXECUTABLE} "${CMAKE_BINARY_DIR}/doc/Doxyfile"
		WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/doc"
		COMMENT "Generating API documentation with Doxygen"
	)
endif ()

function (copy_dlls target target_folder)
	if (WIN32)
		set (assimp_dll "assimp${ASSIMP_LIBRARY_SUFFIX}.dll")
		set (assimp_dll_path "${ASSIMP_ROOT_DIR}/bin/${assimp_dll}")

		if (NOT EXISTS "${target_folder}/${assimp_dll}")
			add_custom_command (
				TARGET ${target}
				POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E copy ${assimp_dll_path} ${target_folder}
				COMMENT "Copying dependent DLLs"
			)
		endif ()
	endif ()
endfunction ()

function (luggcgl_new_assignment assignment_name sources common_sources output_dir)
	add_executable (${assignment_name} ${sources} ${common_sources})

	target_include_directories (
		${assignment_name}
		PRIVATE
			"${CMAKE_SOURCE_DIR}/src"
			"${CMAKE_BINARY_DIR}"
	)
	set_target_properties (
		${assignment_name}
		PROPERTIES
			CXX_STANDARD 14
			CXX_STANDARD_REQUIRED ON
			CXX_EXTENSIONS OFF
	)
	target_link_libraries (${assignment_name} bonobo glm stb::stb tinyfiledialogs::tinyfiledialogs)

	install (TARGETS ${assignment_name} DESTINATION bin)

	copy_dlls (${assignment_name} "${output_dir}")
endfunction ()


# Define groups of files for IDEs
set (PATH_SEP \\)

add_subdirectory ("${CMAKE_SOURCE_DIR}/src/external")
add_subdirectory ("${CMAKE_SOURCE_DIR}/src/core")
add_subdirectory ("${CMAKE_SOURCE_DIR}/src/EDAF80")
add_subdirectory ("${CMAKE_SOURCE_DIR}/src/EDAN35")

install (DIRECTORY ${CMAKE_SOURCE_DIR}/shaders DESTINATION bin)
install (DIRECTORY ${CMAKE_SOURCE_DIR}/res DESTINATION bin)
