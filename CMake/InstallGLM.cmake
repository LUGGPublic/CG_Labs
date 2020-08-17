find_package (glm EXACT QUIET ${LUGGCGL_GLM_DOWNLOAD_VERSION})
if (NOT glm_FOUND)
	set (glm_SOURCE_DIR ${FETCHCONTENT_BASE_DIR}/glm-source)
	set (glm_BINARY_DIR ${FETCHCONTENT_BASE_DIR}/glm-build)
	set (glm_INSTALL_DIR ${FETCHCONTENT_BASE_DIR}/glm-install)

	if (NOT EXISTS ${glm_SOURCE_DIR})
		message (STATUS "Cloning glm…")
		execute_process (
			COMMAND ${GIT_EXECUTABLE} clone --depth=1
			                                -b ${LUGGCGL_GLM_DOWNLOAD_VERSION}
			                                https://github.com/g-truc/glm.git
			                                ${glm_SOURCE_DIR}
			OUTPUT_VARIABLE stdout
			ERROR_VARIABLE stderr
			RESULT_VARIABLE result
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dependencies
		)
		if (result)
			message (FATAL_ERROR "Failed to clone glm: ${result}\n"
			                     "Standard output: ${stdout}\n"
			                     "Error output: ${stderr}")
		endif ()

		# GLM does not set its revision version in its number string,
		# making it impossible to match on it.
		execute_process (
			COMMAND ${GIT_EXECUTABLE} apply ${CMAKE_SOURCE_DIR}/0001-Fix-GLM-version.patch
			OUTPUT_VARIABLE stdout
			ERROR_VARIABLE stderr
			RESULT_VARIABLE result
			WORKING_DIRECTORY "${glm_SOURCE_DIR}"
		)
		if (result)
			message (FATAL_ERROR "Failed to patch glm: ${result}\n"
			                     "Standard output: ${stdout}\n"
			                     "Error output: ${stderr}")
		endif ()

		file (MAKE_DIRECTORY ${glm_BINARY_DIR})
		file (MAKE_DIRECTORY ${glm_INSTALL_DIR})

		message (STATUS "Setting up CMake for glm…")
		execute_process (
			COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}"
			                         -A "${CMAKE_GENERATOR_PLATFORM}"
			                         -DCMAKE_INSTALL_PREFIX=${glm_INSTALL_DIR}
			                         -DCMAKE_BUILD_TYPE=Release
						 -DGLM_TEST_ENABLE=OFF
			                         ${glm_SOURCE_DIR}
			OUTPUT_VARIABLE stdout
			ERROR_VARIABLE stderr
			RESULT_VARIABLE result
			WORKING_DIRECTORY ${glm_BINARY_DIR}
		)
		if (result)
			message (FATAL_ERROR "CMake setup for glm failed: ${result}\n"
			                     "Standard output: ${stdout}\n"
			                     "Error output: ${stderr}")
		endif ()

		message (STATUS "Building and installing glm…")
		execute_process (
			COMMAND ${CMAKE_COMMAND} --build ${glm_BINARY_DIR}
			                         --config Release
			                         --target install
			OUTPUT_VARIABLE stdout
			ERROR_VARIABLE stderr
			RESULT_VARIABLE result
		)
		if (result)
			message (FATAL_ERROR "Build step for glm failed: ${result}\n"
			                     "Standard output: ${stdout}\n"
			                     "Error output: ${stderr}")
		endif ()
	endif ()

	list (APPEND CMAKE_PREFIX_PATH ${glm_INSTALL_DIR}/lib64/cmake)

	set (glm_INSTALL_DIR)
endif ()
