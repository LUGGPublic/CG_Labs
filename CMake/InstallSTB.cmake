FetchContent_Declare (
	stb
	GIT_REPOSITORY [[https://github.com/nothings/stb]]
	GIT_TAG [[b42009b3b9d4ca35bc703f5310eedc74f584be58]]
)

FetchContent_GetProperties (stb)
if (NOT stb_POPULATED)
	message (STATUS "Cloning stb…")
	FetchContent_Populate (stb)
endif ()

add_library( stb::stb INTERFACE IMPORTED)
set_target_properties(stb::stb PROPERTIES
	INTERFACE_INCLUDE_DIRECTORIES "${stb_SOURCE_DIR}"
	INTERFACE_SOURCES "${CMAKE_SOURCE_DIR}/src/core/stb_impl.c"
)
