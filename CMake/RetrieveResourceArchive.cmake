set (resources_SOURCE_FILE ${FETCHCONTENT_BASE_DIR}/resources.zip)

if (NOT EXISTS ${resources_SOURCE_FILE})
	message (STATUS "Downloading resource archive…")
	file (DOWNLOAD
		http://fileadmin.cs.lth.se/cs/Education/EDA221/assignments/EDA221_resources.zip
		${resources_SOURCE_FILE}
		STATUS status
		SHOW_PROGRESS
		EXPECTED_HASH SHA256=c9ae3e0f0b1186b2b9559a642e7db2b64cd28994fc76053c784238f8dddf6388
	)
	list (GET status 0 result)
	if (NOT (result EQUAL 0))
		list (GET status 1 stderr)
		message (FATAL_ERROR "Download step for resources failed: ${result}\n"
		                     "Error output: ${stderr}")
	endif ()

	execute_process (
		COMMAND ${CMAKE_COMMAND} -E tar xf ${resources_SOURCE_FILE}
		OUTPUT_QUIET
		ERROR_VARIABLE stderr
		RESULT_VARIABLE result
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)
	if (result)
		message (FATAL_ERROR "Extraction step for resources failed: ${result}\n"
				     "Error output: ${stderr}")
	endif ()
endif ()
