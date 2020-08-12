add_library (external_libs
	"Dear ImGui/imgui.cpp"
	"Dear ImGui/imgui_demo.cpp"
	"Dear ImGui/imgui_draw.cpp"
	"Dear ImGui/imgui_widgets.cpp"
	"Dear ImGui/imgui_impl_glfw.cpp"
	"Dear ImGui/imgui_impl_opengl3.cpp"

	"GLAD/glad.c"
)

# Include directories
target_include_directories (external_libs SYSTEM PUBLIC "${CMAKE_SOURCE_DIR}/src/external/Dear ImGui")
target_include_directories (external_libs SYSTEM PUBLIC "${CMAKE_SOURCE_DIR}/src/external/GLAD")

# Add dependencies
target_link_libraries (external_libs glfw)
if (NOT WIN32)
	target_link_libraries (external_libs dl)
endif ()

install (TARGETS external_libs DESTINATION lib)
