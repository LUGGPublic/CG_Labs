set (tinyfiledialogs_SOURCE_DIR ${FETCHCONTENT_BASE_DIR}/tinyfiledialogs-source)

if (NOT EXISTS ${tinyfiledialogs_SOURCE_DIR})
	message (STATUS "Cloning tinyfiledialogs…")
	execute_process (
		COMMAND ${GIT_EXECUTABLE} clone --depth=1
						https://git.code.sf.net/p/tinyfiledialogs/code
						${tinyfiledialogs_SOURCE_DIR}
		OUTPUT_QUIET
		ERROR_VARIABLE stderr
		RESULT_VARIABLE result
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dependencies
	)
	if (result)
		message (FATAL_ERROR "Failed to clone assimp: ${result}\n"
				     "Error output: ${stderr}")
	endif ()
endif ()

add_library( tinyfiledialogs::tinyfiledialogs INTERFACE IMPORTED)
set_target_properties(tinyfiledialogs::tinyfiledialogs PROPERTIES
	INTERFACE_INCLUDE_DIRECTORIES "${tinyfiledialogs_SOURCE_DIR}"
	INTERFACE_SOURCES "${tinyfiledialogs_SOURCE_DIR}/tinyfiledialogs.c"
)
