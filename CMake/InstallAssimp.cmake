find_package (assimp QUIET ${LUGGCGL_ASSIMP_MIN_VERSION})
if (NOT assimp_FOUND)
	set (assimp_SOURCE_DIR ${FETCHCONTENT_BASE_DIR}/assimp-source)
	set (assimp_BINARY_DIR ${FETCHCONTENT_BASE_DIR}/assimp-build)

	if (NOT EXISTS ${assimp_SOURCE_DIR})
		message (STATUS "Cloning assimp…")
		execute_process (
			COMMAND ${GIT_EXECUTABLE} clone --depth=1
			                                -b v${LUGGCGL_ASSIMP_MIN_VERSION}
							https://github.com/assimp/assimp.git
							${assimp_SOURCE_DIR}
			OUTPUT_QUIET
			ERROR_VARIABLE stderr
			RESULT_VARIABLE result
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dependencies
		)
		if (result)
			message (FATAL_ERROR "Failed to clone assimp: ${result}\n"
			                     "Error output: ${stderr}")
		endif ()

		set (assimp_INSTALL_DIR ${FETCHCONTENT_BASE_DIR}/assimp-install)
		file (MAKE_DIRECTORY ${assimp_BINARY_DIR})
		file (MAKE_DIRECTORY ${assimp_INSTALL_DIR})

		message (STATUS "Setting up CMake for assimp…")
		execute_process (
			COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}"
			                         -DASSIMP_NO_EXPORT=ON
			                         -DASSIMP_BUILD_ASSIMP_TOOLS=OFF
			                         -DASSIMP_BUILD_TESTS=OFF
			                         -DCMAKE_INSTALL_PREFIX=${assimp_INSTALL_DIR}
			                         -DCMAKE_BUILD_TYPE=Release
			                         ${assimp_SOURCE_DIR}
			OUTPUT_QUIET
			ERROR_VARIABLE stderr
			RESULT_VARIABLE result
			WORKING_DIRECTORY ${assimp_BINARY_DIR}
		)
		if (result)
			message (FATAL_ERROR "CMake setup for assimp failed: ${result}\n"
			                     "Error output: ${stderr}")
		endif ()

		message (STATUS "Building and installing assimp…")
		execute_process (
			COMMAND ${CMAKE_COMMAND} --build ${assimp_BINARY_DIR}
			                         --config Release
			                         --target install
			OUTPUT_QUIET
			ERROR_VARIABLE stderr
			RESULT_VARIABLE result
		)
		if (result)
			message (FATAL_ERROR "Build step for assimp failed: ${result}\n"
			                     "Error output: ${stderr}")
		endif ()

		list (APPEND CMAKE_PREFIX_PATH ${assimp_INSTALL_DIR}/lib/cmake)

		set (assimp_INSTALL_DIR)
	endif ()
endif ()
