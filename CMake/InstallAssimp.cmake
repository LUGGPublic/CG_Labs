find_package (assimp QUIET ${LUGGCGL_ASSIMP_MIN_VERSION})
if (NOT assimp_FOUND)
	FetchContent_Declare (
		assimp
		GIT_REPOSITORY [[https://github.com/assimp/assimp.git]]
		GIT_TAG "v${LUGGCGL_ASSIMP_DOWNLOAD_VERSION}"
		GIT_SHALLOW ON
		SOURCE_SUBDIR not-a-cmake-project
	)

	message (STATUS "Fetching assimp sources…")
	FetchContent_MakeAvailable (assimp)

	set (assimp_INSTALL_DIR "${FETCHCONTENT_BASE_DIR}/assimp-install")
	if (NOT EXISTS "${assimp_INSTALL_DIR}")
		file (MAKE_DIRECTORY ${assimp_INSTALL_DIR})
	endif ()

	# assimp bundles a very old zlib whose zutil.h defines fdopen() to NULL when
	# TARGET_OS_MAC is visible, which breaks <stdio.h> on recent macOS SDKs.
	# macOS ships zlib, so let assimp find that one instead of building its own.
	if (APPLE)
		set (LUGGCGL_ASSIMP_BUILD_ZLIB OFF)
	else ()
		set (LUGGCGL_ASSIMP_BUILD_ZLIB ON)
	endif ()

	message (STATUS "Setting up CMake for assimp…")
	execute_process (
		COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}"
		                         -A "${CMAKE_GENERATOR_PLATFORM}"
		                         -DASSIMP_NO_EXPORT=ON
		                         -DASSIMP_BUILD_ASSIMP_TOOLS=OFF
		                         -DASSIMP_BUILD_ZLIB=${LUGGCGL_ASSIMP_BUILD_ZLIB}
		                         -DASSIMP_BUILD_TESTS=OFF
		                         -DASSIMP_WARNINGS_AS_ERRORS=OFF
		                         -DCMAKE_INSTALL_PREFIX=${assimp_INSTALL_DIR}
		                         -DCMAKE_BUILD_TYPE=Release
		                         ${assimp_SOURCE_DIR}
		OUTPUT_VARIABLE stdout
		ERROR_VARIABLE stderr
		RESULT_VARIABLE result
		WORKING_DIRECTORY ${assimp_BINARY_DIR}
	)
	if (result)
		message (FATAL_ERROR "CMake setup for assimp failed: ${result}\n"
		                     "Standard output: ${stdout}\n"
		                     "Error output: ${stderr}")
	endif ()

	message (STATUS "Building and installing assimp…")
	execute_process (
		COMMAND ${CMAKE_COMMAND} --build ${assimp_BINARY_DIR}
		                         --config Release
		                         --target install
		RESULT_VARIABLE result
	)
	if (result)
		message (FATAL_ERROR "Build step for assimp failed: ${result}")
	endif ()

	list (APPEND CMAKE_PREFIX_PATH ${assimp_INSTALL_DIR})

	set (assimp_INSTALL_DIR)
endif ()
