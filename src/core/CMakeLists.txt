add_library (bonobo
	"Bonobo.cpp"
	"GLStateInspection.cpp"
	"GLStateInspectionView.cpp"
	"InputHandler.cpp"
	"Log.cpp"
	"LogView.cpp"
	"Misc.cpp"
	"opengl.cpp"
	"ShaderProgramManager.cpp"
	"various.cpp"
	"WindowManager.cpp"

	"node.cpp"
	"node.hpp"
	"helpers.cpp"
	"helpers.hpp"
)

target_include_directories (
	bonobo
	PRIVATE
		"${CMAKE_SOURCE_DIR}/src"
		"${CMAKE_BINARY_DIR}"
	PUBLIC
		"${ASSIMP_INCLUDE_DIRS}"
)

target_link_libraries (
	bonobo
	PUBLIC
		${ASSIMP_LIBRARIES}
		external_libs
		glfw
		glm
	PRIVATE
		CG_Labs_options
		stb::stb
)

install (TARGETS bonobo DESTINATION lib)
