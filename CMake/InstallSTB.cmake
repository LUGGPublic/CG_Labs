set (stb_SOURCE_DIR ${FETCHCONTENT_BASE_DIR}/stb-source)

if (NOT EXISTS ${stb_SOURCE_DIR})
	message (STATUS "Cloning stb…")
	execute_process (
		COMMAND ${GIT_EXECUTABLE} clone --depth=1
		                                https://github.com/nothings/stb
		                                ${stb_SOURCE_DIR}
		OUTPUT_QUIET
		ERROR_VARIABLE stderr
		RESULT_VARIABLE result
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dependencies
	)
	if (result)
		message (FATAL_ERROR "Failed to clone stb: ${result}\n"
		                     "Error output: ${stderr}")
	endif ()
endif ()

add_library( stb::stb INTERFACE IMPORTED)
set_target_properties(stb::stb PROPERTIES
	INTERFACE_INCLUDE_DIRECTORIES "${stb_SOURCE_DIR}"
	INTERFACE_SOURCES "${CMAKE_SOURCE_DIR}/src/core/stb_impl.c"
)
