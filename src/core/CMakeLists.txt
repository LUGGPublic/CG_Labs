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
	"Types.cpp"
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

target_link_libraries (bonobo ${ASSIMP_LIBRARIES} CG_Labs_options external_libs glfw glm stb::stb)

install (TARGETS bonobo DESTINATION lib)
