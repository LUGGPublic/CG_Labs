FetchContent_Declare (
	resources
	URL [[http://fileadmin.cs.lth.se/cs/Education/EDA221/assignments/EDA221_resources.zip]]
	URL_HASH [[SHA256=c9ae3e0f0b1186b2b9559a642e7db2b64cd28994fc76053c784238f8dddf6388]]
	SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/res"
)

FetchContent_GetProperties (resources)
if (NOT resources_POPULATED)
	message (STATUS "Downloading resource archive…")
	FetchContent_Populate (resources)
endif ()
