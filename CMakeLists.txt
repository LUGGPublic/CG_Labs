# As we use the VERSION option---which was introduced in CMAKE 3.0---of the
# project() command, set the minimum required version to 3.0.
cmake_minimum_required (VERSION 3.0)

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


if (WIN32)
	set (BUILD_SHARED_LIBS OFF)
endif ()


# OpenGL is used for the rendering
find_package (OpenGL REQUIRED)


# assimp is used for loading the objects’s models
set (LUGGCGL_MIN_ASSIMP_VERSION 4.0.1)
find_package (assimp QUIET ${LUGGCGL_MIN_ASSIMP_VERSION})
if (NOT assimp_FOUND)
	if (NOT EXISTS "${CMAKE_BINARY_DIR}/assimp")
		file (MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/assimp")
	endif ()
	if (NOT EXISTS "${CMAKE_BINARY_DIR}/assimp/code")
		message (STATUS "As ASSIMP >= ${LUGGCGL_MIN_ASSIMP_VERSION} was not found on your computer, we will fetch it from the source.")

		find_package (Git REQUIRED)
		execute_process (
			COMMAND ${GIT_EXECUTABLE} clone -b "v${LUGGCGL_MIN_ASSIMP_VERSION}" "https://github.com/assimp/assimp.git" "code"
			WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/assimp"
		)
		# assimp will not compile under VS2017.3 without that commit,
		# which is not part of a release yet.
		execute_process (
			COMMAND ${GIT_EXECUTABLE} apply ${CMAKE_SOURCE_DIR}/0001-fix-compiling-problems-under-vs2017-3.patch
			WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/assimp/code"
		)
	endif ()
	if (NOT EXISTS "${CMAKE_BINARY_DIR}/assimp/build")
		file (MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/assimp/build")
	endif ()
	set (ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE)
	set (ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
	set (ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
	add_subdirectory ("${CMAKE_BINARY_DIR}/assimp/code" "${CMAKE_BINARY_DIR}/assimp/binary")
	set (USE_SYSTEM_ASSIMP FALSE)
	set (ASSIMP_INCLUDE_DIRS "${CMAKE_BINARY_DIR}/assimp/code/include" "${CMAKE_BINARY_DIR}/assimp/binary/include")
endif ()


if (NOT (DEFINED USE_SYSTEM_ASSIMP))
	set (USE_SYSTEM_ASSIMP TRUE)
endif ()
if (NOT (DEFINED USE_SYSTEM_GLFW))
	set (USE_SYSTEM_GLFW TRUE)
endif ()
if (NOT (DEFINED USE_SYSTEM_IMGUI))
	set (USE_SYSTEM_IMGUI TRUE)
endif ()
if (NOT (DEFINED USE_SYSTEM_GLM))
	set (USE_SYSTEM_GLM TRUE)
endif ()


# GLFW is used for inputs and windows handling
set (LUGGCGL_MIN_GLFW_VERSION 3.2.1)
find_package (glfw3 QUIET ${LUGGCGL_MIN_GLFW_VERSION})
if (NOT glfw3_FOUND)
	if (NOT EXISTS "${CMAKE_BINARY_DIR}/glfw")
		file (MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/glfw")
	endif ()
	if (NOT EXISTS "${CMAKE_BINARY_DIR}/glfw/code")
		message (STATUS "As GLFW >= ${LUGGCGL_MIN_GLFW_VERSION} was not found on your computer, we will fetch it from the source.")

		find_package (Git REQUIRED)
		execute_process (
			COMMAND ${GIT_EXECUTABLE} clone -b "${LUGGCGL_MIN_GLFW_VERSION}" "https://github.com/glfw/glfw.git" "code"
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
endif ()


# GLM is used for matrices, vectors and camera handling
set (LUGGCGL_MIN_GLM_VERSION 0.9.8.5)
find_package (glm QUIET ${LUGGCGL_MIN_GLM_VERSION})
if (NOT glm_FOUND)
	MESSAGE (STATUS "As GLM >= ${LUGGCGL_MIN_GLM_VERSION} was not found on your computer, we will fetch it from the source.")
	EXTERNALPROJECT_ADD (
		glm
		PREFIX "glm"
		GIT_REPOSITORY "https://github.com/g-truc/glm"
		GIT_TAG ${LUGGCGL_MIN_GLM_VERSION}
		BUILD_COMMAND ""
		INSTALL_COMMAND ""
	)
	set (USE_SYSTEM_GLM FALSE)
	set (GLM_INCLUDE_DIRS "${CMAKE_BINARY_DIR}/glm/src/glm" CACHE STRING "glm include dir")
endif ()


# ImGui is used for the windowing system inside the application
set (LUGGCGL_MIN_IMGUI_VERSION 1.51)
find_package (imgui QUIET ${LUGGCGL_MIN_IMGUI_VERSION})
if (NOT imgui_FOUND)
	if (NOT EXISTS "${CMAKE_BINARY_DIR}/imgui")
		file (MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/imgui")
	endif ()
	if (NOT EXISTS "${CMAKE_BINARY_DIR}/imgui/code")
		message (STATUS "As Dear ImGUI >= ${LUGGCGL_MIN_IMGUI_VERSION} was not found on your computer, we will fetch it from the source.")

		find_package (Git REQUIRED)
		execute_process (
			COMMAND ${GIT_EXECUTABLE} clone -b "v${LUGGCGL_MIN_IMGUI_VERSION}" "https://github.com/ocornut/imgui.git" "code"
			WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/imgui"
		)
		execute_process (
			COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/Dear_ImGui_CMakeLists.txt code/CMakeLists.txt
			WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/imgui"
		)
	endif ()
	if (NOT EXISTS "${CMAKE_BINARY_DIR}/imgui/build")
		file (MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/imgui/build")
	endif ()
	add_subdirectory ("${CMAKE_BINARY_DIR}/imgui/code" "${CMAKE_BINARY_DIR}/imgui/binary")
	set (USE_SYSTEM_IMGUI FALSE)
	set (IMGUI_INCLUDE_DIRS "${CMAKE_BINARY_DIR}/imgui/code")
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
set (USE_SYSTEM_GLM ${USE_SYSTEM_GLM} CACHE BOOL "Whether to use the system version of GLM")
set (USE_SYSTEM_IMGUI ${USE_SYSTEM_IMGUI} CACHE BOOL "Whether to use the system version of Dear ImGUI")


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

function (luggcgl_new_assignment assignment_name sources common_sources)
	add_executable (${assignment_name} ${sources} ${common_sources})

	target_include_directories (${assignment_name} PRIVATE "${CMAKE_SOURCE_DIR}/src/external")
	target_include_directories (${assignment_name} PRIVATE "${CMAKE_SOURCE_DIR}/src")
	target_include_directories (${assignment_name} PRIVATE "${CMAKE_BINARY_DIR}")
	target_include_directories (${assignment_name} PRIVATE ${IMGUI_INCLUDE_DIRS})
	target_include_directories (${assignment_name} PRIVATE ${GLM_INCLUDE_DIRS})

	set_property (TARGET ${assignment_name} PROPERTY CXX_STANDARD 14)
	set_property (TARGET ${assignment_name} PROPERTY CXX_STANDARD_REQUIRED ON)
	set_property (TARGET ${assignment_name} PROPERTY CXX_EXTENSIONS OFF)

	set_target_properties (${assignment_name} PROPERTIES
		INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib
		INSTALL_RPATH_USE_LINK_PATH TRUE)

	add_dependencies (${assignment_name} bonobo)

	target_link_libraries (${assignment_name} bonobo ${OPENGL_gl_LIBRARY} imgui ${LUGGCGL_EXTRA_LIBS})

	install (TARGETS ${assignment_name} DESTINATION bin)
endfunction ()


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
add_subdirectory ("${CMAKE_SOURCE_DIR}/src/EDAF80")
add_subdirectory ("${CMAKE_SOURCE_DIR}/src/EDAN35")

install (DIRECTORY ${CMAKE_SOURCE_DIR}/shaders DESTINATION bin)
install (DIRECTORY ${CMAKE_SOURCE_DIR}/res DESTINATION bin)
