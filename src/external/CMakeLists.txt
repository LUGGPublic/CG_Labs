set (
	SOURCES

	"glad.c"
	"imgui_impl_glfw_gl3.cpp"
	"lodepng.cpp"
)

add_library (external_libs ${SOURCES})

# Include directories
target_include_directories (external_libs PRIVATE ${IMGUI_INCLUDE_DIRS})
target_include_directories (external_libs PRIVATE "${PROJECT_SOURCE_DIR}/src/external")

target_link_libraries (external_libs glfw ${LUGGCGL_EXTRA_LIBS})
