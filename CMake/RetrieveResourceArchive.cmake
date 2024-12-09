FetchContent_Declare (
	resources
	URL [[http://fileadmin.cs.lth.se/cs/Education/EDAF80/assignments/resources.zip]]
	URL_HASH [[SHA512=832b8e45973705b69eeae61be75dcc444949c53d8656c483664896b9e8daadce7f826b244b93bc258d0d3678721d4ab7d42bcaae02703590458ad25663d647cf]]
	SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/res"
)

FetchContent_GetProperties (resources)
if (NOT resources_POPULATED)
	message (STATUS "Downloading resource archive…")
	FetchContent_Populate (resources)
endif ()
