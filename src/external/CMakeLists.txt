add_library (external_libs
	"GLAD/glad.c"
	"imgui_impl_glfw_gl3.cpp"
)

# Include directories
target_include_directories (external_libs SYSTEM PRIVATE "${CMAKE_SOURCE_DIR}/src/external")
target_include_directories (external_libs SYSTEM PUBLIC "${CMAKE_SOURCE_DIR}/src/external/GLAD")

# Add dependencies
target_link_libraries (external_libs glfw imgui::imgui)
if (NOT WIN32)
	target_link_libraries (external_libs dl)
endif ()

install (TARGETS external_libs DESTINATION lib)
