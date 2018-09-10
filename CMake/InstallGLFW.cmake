find_package (glfw3 QUIET ${LUGGCGL_GLFW_MIN_VERSION})
if (NOT glfw3_FOUND)
	set (glfw_SOURCE_DIR ${FETCHCONTENT_BASE_DIR}/glfw-source)
	set (glfw_BINARY_DIR ${FETCHCONTENT_BASE_DIR}/glfw-build)

	if (NOT EXISTS ${glfw_SOURCE_DIR})
		message (STATUS "Cloning glfw…")
		execute_process (
			COMMAND ${GIT_EXECUTABLE} clone --depth=1
			                                -b ${LUGGCGL_GLFW_MIN_VERSION}
							https://github.com/glfw/glfw.git
							${glfw_SOURCE_DIR}
			OUTPUT_QUIET
			ERROR_VARIABLE stderr
			RESULT_VARIABLE result
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dependencies
		)
		if (result)
			message (FATAL_ERROR "Failed to clone glfw: ${result}\n"
			                     "Error output: ${stderr}")
		endif ()

		set (glfw_INSTALL_DIR ${FETCHCONTENT_BASE_DIR}/glfw-install)
		file (MAKE_DIRECTORY ${glfw_BINARY_DIR})
		file (MAKE_DIRECTORY ${glfw_INSTALL_DIR})

		message (STATUS "Setting up CMake for glfw…")
		execute_process (
			COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}"
			                         -DGLFW_BUILD_DOCS=OFF
			                         -DGLFW_BUILD_TESTS=OFF
			                         -DGLFW_BUILD_EXAMPLES=OFF
			                         -DCMAKE_INSTALL_PREFIX=${glfw_INSTALL_DIR}
			                         -DCMAKE_BUILD_TYPE=Release
			                         ${glfw_SOURCE_DIR}
			OUTPUT_QUIET
			ERROR_VARIABLE stderr
			RESULT_VARIABLE result
			WORKING_DIRECTORY ${glfw_BINARY_DIR}
		)
		if (result)
			message (FATAL_ERROR "CMake setup for glfw failed: ${result}\n"
			                     "Error output: ${stderr}")
		endif ()

		message (STATUS "Building and installing glfw…")
		execute_process (
			COMMAND ${CMAKE_COMMAND} --build ${glfw_BINARY_DIR}
			                         --config Release
			                         --target install
			OUTPUT_QUIET
			ERROR_VARIABLE stderr
			RESULT_VARIABLE result
		)
		if (result)
			message (FATAL_ERROR "Build step for glfw failed: ${result}\n"
			                     "Error output: ${stderr}")
		endif ()

		list (APPEND CMAKE_PREFIX_PATH ${glfw_INSTALL_DIR}/lib/cmake)

		set (glfw_INSTALL_DIR)
	endif ()
endif ()
