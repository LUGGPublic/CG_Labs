set (
	SOURCES

	"glad.c"
	"imgui_impl_glfw_gl3.cpp"
	"lodepng.cpp"
)

add_library (external_libs ${SOURCES})

# Include directories
target_include_directories (external_libs PRIVATE ${IMGUI_INCLUDE_DIRS})
target_include_directories (external_libs PRIVATE "${CMAKE_SOURCE_DIR}/src/external")

set_target_properties (external_libs PROPERTIES
	INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib
	INSTALL_RPATH_USE_LINK_PATH TRUE)

target_link_libraries (external_libs glfw imgui ${LUGGCGL_EXTRA_LIBS})

install (TARGETS external_libs DESTINATION lib)
