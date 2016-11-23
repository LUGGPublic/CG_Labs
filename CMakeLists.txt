# As we use the VERSION option---which was introduced in CMAKE 3.0---of the
# project() command, set the minimum required version to 3.0.
cmake_minimum_required (VERSION 3.0)


# Set the project name, versions and languages used.
project (
	LU_Computer_Graphics_Labs
	VERSION 0.1
	LANGUAGES CXX C
)


# To be able to dowload and compile the dependencies
include (ExternalProject)


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


# OpenGL is used for the rendering
find_package (OpenGL REQUIRED)


# assimp is used for loading the objects’s models
# 3.3 is required as it is the first version to have (experimental) glTF
# support
find_package (assimp QUIET)
if (NOT assimp_FOUND)
	message (STATUS "As assimp was not found on your computer, we will fetch it from the source.")
	EXTERNALPROJECT_ADD (
		assimp
		PREFIX "assimp"
		GIT_REPOSITORY "https://github.com/assimp/assimp.git"
		GIT_TAG v3.3.1
		BINARY_DIR "assimp/build"
		CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> -DASSIMP_NO_EXPORT=ON -DASSIMP_BUILD_ASSIMP_TOOLS=OFF -DASSIMP_BUILD_TESTS=OFF
	)
	set (USE_SYSTEM_ASSIMP FALSE)
	set (assimp_FOUND TRUE CACHE BOOL "Whether assimp was found")
	set (ASSIMP_INCLUDE_DIRS "${CMAKE_BINARY_DIR}/assimp/include" CACHE STRING "assimp include dir")
	if (CMAKE_HOST_WIN32)
		string (REGEX REPLACE "v([0-9][0-9])([0-9])" "vc\\1\\2" VS_TOOLSET_VERSION "${CMAKE_VS_PLATFORM_TOOLSET}" )
		set (ASSIMP_LIBRARIES "${CMAKE_BINARY_DIR}/assimp/lib/assimp-${VS_TOOLSET_VERSION}-mt.lib" CACHE STRING "assimp lib path")
		set (ASSIMP_DLL_PATH "${CMAKE_BINARY_DIR}/assimp/bin/assimp-${VS_TOOLSET_VERSION}-mt.dll" CACHE STRING "assimp dll path")
		file (MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/src/EDA221/Debug)
		file (MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/src/EDA221/Release)
		add_custom_command (TARGET assimp POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${ASSIMP_DLL_PATH} ${CMAKE_BINARY_DIR}/src/EDA221/Debug)
		add_custom_command (TARGET assimp POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${ASSIMP_DLL_PATH} ${CMAKE_BINARY_DIR}/src/EDA221/Release)
		file (MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/src/EDAN35/Debug)
		file (MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/src/EDAN35/Release)
		add_custom_command (TARGET assimp POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${ASSIMP_DLL_PATH} ${CMAKE_BINARY_DIR}/src/EDAN35/Debug)
		add_custom_command (TARGET assimp POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${ASSIMP_DLL_PATH} ${CMAKE_BINARY_DIR}/src/EDAN35/Release)
	elseif (CMAKE_HOST_APPLE)
		set (ASSIMP_LIBRARIES "${CMAKE_BINARY_DIR}/assimp/lib/libassimp.dylib" CACHE STRING "assimp lib path")
	else ()
		set (ASSIMP_LIBRARIES "${CMAKE_BINARY_DIR}/assimp/lib/libassimp.so" CACHE STRING "assimp lib path")
	endif ()
endif ()


if (NOT (DEFINED USE_SYSTEM_ASSIMP))
	set (USE_SYSTEM_ASSIMP TRUE)
endif ()
if (NOT (DEFINED USE_SYSTEM_GLFW))
	set (USE_SYSTEM_GLFW TRUE)
endif ()
if (NOT (DEFINED USE_SYSTEM_GLM))
	set (USE_SYSTEM_GLM TRUE)
endif ()


# GLFW is used for inputs and windows handling
find_package (glfw3 QUIET)
if (NOT glfw3_FOUND)
	if (NOT EXISTS "${CMAKE_BINARY_DIR}/glfw")
		file (MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/glfw")
	endif ()
	if (NOT EXISTS "${CMAKE_BINARY_DIR}/glfw/code")
		message (STATUS "As GLFW was not found on your computer, we will fetch it from the source.")

		find_package (Git REQUIRED)
		execute_process (
			COMMAND ${GIT_EXECUTABLE} clone -b "3.2.1" "https://github.com/glfw/glfw.git" "code"
			WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/glfw"
		)
	endif ()
	if (NOT EXISTS "${CMAKE_BINARY_DIR}/glfw/build")
		file (MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/glfw/build")
	endif ()
	set (GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
	set (GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
	set (GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
	add_subdirectory ("${CMAKE_BINARY_DIR}/glfw/code" "${CMAKE_BINARY_DIR}/glfw/binary")
	set (USE_SYSTEM_GLFW FALSE)
	set (glfw_FOUND TRUE CACHE BOOL "Whether glfw was found")
endif ()


# GLM is used for matrices, vectors and camera handling
find_package (glm QUIET)
if (NOT glm_FOUND)
	MESSAGE (STATUS "As GLM was not found on your computer, we will fetch it from the source.")
	EXTERNALPROJECT_ADD (
		glm
		PREFIX "glm"
		GIT_REPOSITORY "https://github.com/g-truc/glm"
		GIT_TAG 0.9.7.1
		BUILD_COMMAND ""
		INSTALL_COMMAND ""
	)
	set (USE_SYSTEM_GLM FALSE)
	set (glm_FOUND TRUE CACHE BOOL "Whether glm was found")
	set (GLM_INCLUDE_DIRS "${CMAKE_BINARY_DIR}/glm/src/glm" CACHE STRING "glm include dir")
endif ()


# ImGui is used for the windowing system inside the application
find_package (imgui QUIET)
if (NOT imgui_FOUND)
	MESSAGE (STATUS "As Dear ImGUI was not found on your computer, we will fetch it from the source.")
	EXTERNALPROJECT_ADD (
		imgui
		PREFIX "imgui"
		GIT_REPOSITORY "https://github.com/ocornut/imgui.git"
		GIT_TAG v1.46
		BINARY_DIR "imgui/build"
		UPDATE_COMMAND ${CMAKE_COMMAND} -E copy
		${PROJECT_SOURCE_DIR}/Dear_ImGui_CMakeLists.txt
		${CMAKE_BINARY_DIR}/imgui/src/imgui/CMakeLists.txt
		CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
	)
	set (IMGUI_INCLUDE_DIRS "${CMAKE_BINARY_DIR}/imgui/include")
	if (BUILD_SHARED_LIBS)
		set (IMGUI_LIBRARY "${CMAKE_BINARY_DIR}/imgui/lib/${CMAKE_SHARED_LIBRARY_PREFIX}imgui${CMAKE_SHARED_LIBRARY_SUFFIX}")
	else ()
		set (IMGUI_LIBRARY "${CMAKE_BINARY_DIR}/imgui/lib/${CMAKE_STATIC_LIBRARY_PREFIX}imgui${CMAKE_STATIC_LIBRARY_SUFFIX}")
	endif ()
endif ()

set (LUCGL_RES_ZIP_FILE "${CMAKE_SOURCE_DIR}/res.zip")
if (NOT EXISTS "${LUCGL_RES_ZIP_FILE}")
	message (STATUS "The resources folder was not found on your computer: downloading it.")
	file (
		DOWNLOAD
		"http://fileadmin.cs.lth.se/cs/Education/EDA221/assignments/EDA221_resources.zip"
		"${LUCGL_RES_ZIP_FILE}"
		STATUS RES_DL_STATUS
		SHOW_PROGRESS
	)
	list (GET RES_DL_STATUS 0 RES_DL_CODE)
	list (GET RES_DL_STATUS 1 RES_DL_STRING)
	if (NOT (RES_DL_STATUS EQUAL 0))
		message (WARNING "Could not download the zip file containing the resources: ${RES_DL_STRING}")
	endif ()
endif ()
if ((NOT EXISTS "${CMAKE_SOURCE_DIR}/res") AND (EXISTS "${LUCGL_RES_ZIP_FILE}"))
	execute_process(
		COMMAND ${CMAKE_COMMAND} -E tar xf "${LUCGL_RES_ZIP_FILE}"
		WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
	)
endif ()


set (USE_SYSTEM_ASSIMP ${USE_SYSTEM_ASSIMP} CACHE BOOL "Whether to use the system version of ASSIMP")
set (USE_SYSTEM_GLFW ${USE_SYSTEM_GLFW} CACHE BOOL "Whether to use the system version of GLFW")
#set (USE_SYSTEM_GLM ${USE_SYSTEM_GLM} CACHE BOOL "Whether to use the system version of GLFW")


# Configure *C++ Environment Variables*
set (MSAA_RATE "1" CACHE STRING "Window MSAA rate")
set (WIDTH "1600" CACHE STRING "Window width")
set (HEIGHT "900" CACHE STRING "Window height")
set (SHADERS_DIR "${PROJECT_SOURCE_DIR}/shaders")
set (RESOURCES_DIR "${PROJECT_SOURCE_DIR}/res")
configure_file ("${PROJECT_SOURCE_DIR}/src/core/config.hpp.in" "${PROJECT_BINARY_DIR}/config.hpp")


# Set up Doxygen documentation generation
option (LUGGCGL_BUILD_DOCUMENTATION "Build documentation for Lund University Computer Graphics Labs" OFF)
if (LUGGCGL_BUILD_DOCUMENTATION)
	find_package (Doxygen REQUIRED)
	set (LUGGCGL_VERSION_NUMBER "${CMAKE_PROJECT_VERSION}")
	set (LUGGCGL_DOC_INPUT "${CMAKE_SOURCE_DIR}/src")
	set (LUGGCGL_DOC_EXCLUDE ${CMAKE_SOURCE_DIR}/src/external" "${CMAKE_SOURCE_DIR}/src/EDA221_Assignment1)
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


# Define groups of files for IDEs
set (PATH_SEP \\)
source_group (Source\ Files${PATH_SEP}External${PATH_SEP}lodepng FILES ${PROJECT_SOURCE_DIR}/src/lodepng.cpp ${PROJECT_SOURCE_DIR}/src/external/lodepng.h)
source_group (Source\ Files${PATH_SEP}External${PATH_SEP}GLFW FILES ${PROJECT_SOURCE_DIR}/src/external/imgui_impl_glfw_gl3.cpp ${PROJECT_SOURCE_DIR}/src/external/imgui_impl_glfw_gl3.h)
source_group (Source\ Files${PATH_SEP}External${PATH_SEP}GL3W FILES ${PROJECT_SOURCE_DIR}/src/external/gl3w.cpp)


add_custom_target (project_dep ALL)
if (NOT USE_SYSTEM_ASSIMP)
	add_dependencies (project_dep assimp)
endif ()
if (NOT USE_SYSTEM_GLM)
	add_dependencies (project_dep glm)
endif ()
add_dependencies (project_dep imgui)

if (APPLE)
	find_library (COCOA_LIBRARY Cocoa)
	find_library (IOKIT_LIBRARY IOKit)
	find_library (CORE_VIDEO_LIBRARY CoreVideo)
	find_library (CORE_FOUNDATION_LIBRARY CoreFoundation)
	set (LUGGCGL_EXTRA_LIBS ${COCOA_LIBRARY} ${IOKIT_LIBRARY} ${CORE_VIDEO_LIBRARY} ${CORE_FOUNDATION_LIBRARY})
elseif (UNIX)
	set (LUGGCGL_EXTRA_LIBS dl)
endif ()

add_subdirectory ("${CMAKE_SOURCE_DIR}/src/external")
add_subdirectory ("${CMAKE_SOURCE_DIR}/src/core")
add_dependencies (bonobo project_dep)
add_dependencies (external_libs project_dep)
add_dependencies (bonobo external_libs)
add_subdirectory ("${CMAKE_SOURCE_DIR}/src/EDA221_Assignment1")
add_subdirectory ("${CMAKE_SOURCE_DIR}/src/EDA221")
add_subdirectory ("${CMAKE_SOURCE_DIR}/src/EDAN35")
