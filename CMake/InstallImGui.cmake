set (imgui_SOURCE_DIR ${FETCHCONTENT_BASE_DIR}/imgui-source)
set (imgui_BINARY_DIR ${FETCHCONTENT_BASE_DIR}/imgui-build)

if (NOT EXISTS ${imgui_SOURCE_DIR})
	message (STATUS "Cloning imgui…")
	execute_process (
		COMMAND ${GIT_EXECUTABLE} clone --depth=1
						-b v${LUGGCGL_IMGUI_MIN_VERSION}
						https://github.com/ocornut/imgui.git
						${imgui_SOURCE_DIR}
		OUTPUT_QUIET
		ERROR_VARIABLE stderr
		RESULT_VARIABLE result
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dependencies
	)
	if (result)
		message (FATAL_ERROR "Failed to clone imgui: ${result}\n"
				     "Error output: ${stderr}")
	endif ()

	set (imgui_INSTALL_DIR ${FETCHCONTENT_BASE_DIR}/imgui-install)
	file (MAKE_DIRECTORY ${imgui_BINARY_DIR})
	file (MAKE_DIRECTORY ${imgui_INSTALL_DIR})

	execute_process (
		COMMAND ${CMAKE_COMMAND} -E copy
		                         ${PROJECT_SOURCE_DIR}/Dear_ImGui_CMakeLists.txt
		                         ${imgui_SOURCE_DIR}/CMakeLists.txt
		OUTPUT_QUIET
		ERROR_VARIABLE stderr
		RESULT_VARIABLE result
		WORKING_DIRECTORY "${imgui_BINARY_DIR}"
	)
	if (result)
		message (FATAL_ERROR "Copying custom CMake file for ImGui failed: ${result}\n"
		                     "Error output: ${stderr}")
	endif ()

	execute_process (
		COMMAND ${CMAKE_COMMAND} -E copy
		                         ${PROJECT_SOURCE_DIR}/Dear_ImGui_imguiConfig.cmake
		                         ${imgui_SOURCE_DIR}/imguiConfig.cmake
		OUTPUT_QUIET
		ERROR_VARIABLE stderr
		RESULT_VARIABLE result
		WORKING_DIRECTORY "${imgui_BINARY_DIR}"
	)
	if (result)
		message (FATAL_ERROR "Copying custom CMake config file for ImGui failed: ${result}\n"
		                     "Error output: ${stderr}")
	endif ()

	message (STATUS "Setting up CMake for imgui…")
	execute_process (
		COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}"
		                         -DCMAKE_INSTALL_PREFIX=${imgui_INSTALL_DIR}
		                         -DCMAKE_BUILD_TYPE=Release
		                         ${imgui_SOURCE_DIR}
		OUTPUT_QUIET
		ERROR_VARIABLE stderr
		RESULT_VARIABLE result
		WORKING_DIRECTORY ${imgui_BINARY_DIR}
	)
	if (result)
		message (FATAL_ERROR "CMake setup for imgui failed: ${result}\n"
		                     "Error output: ${stderr}")
	endif ()

	message (STATUS "Building and installing imgui…")
	execute_process (
		COMMAND ${CMAKE_COMMAND} --build ${imgui_BINARY_DIR}
		                         --config Release
		                         --target install
		OUTPUT_QUIET
		ERROR_VARIABLE stderr
		RESULT_VARIABLE result
	)
	if (result)
		message (FATAL_ERROR "Build step for imgui failed: ${result}\n"
		                     "Error output: ${stderr}")
	endif ()

	list (APPEND CMAKE_PREFIX_PATH ${imgui_INSTALL_DIR}/lib/cmake)
	set (IMGUI_INCLUDE_DIRS ${imgui_INSTALL_DIR}/include)

	set (imgui_INSTALL_DIR)
endif ()
