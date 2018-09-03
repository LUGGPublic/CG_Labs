cmake_minimum_required (VERSION 3.0)

project (bonobo
	VERSION 0.2
	LANGUAGES CXX
)

add_library (${PROJECT_NAME}
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
	${PROJECT_NAME}
	PRIVATE
		"${CMAKE_SOURCE_DIR}/src"
		"${CMAKE_BINARY_DIR}"
)

target_include_directories (
	${PROJECT_NAME}
	SYSTEM PRIVATE
		${ASSIMP_INCLUDE_DIRS}
		"${CMAKE_SOURCE_DIR}/src/external"
)

set_target_properties (
	${PROJECT_NAME}
	PROPERTIES
		CXX_STANDARD 14
		CXX_STANDARD_REQUIRED ON
		CXX_EXTENSIONS OFF
)
target_link_libraries (${PROJECT_NAME} imgui::imgui external_libs glfw glm ${ASSIMP_LIBRARIES})

install (TARGETS ${PROJECT_NAME} DESTINATION lib)
