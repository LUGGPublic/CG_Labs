cmake_minimum_required (VERSION 3.0)

project (bonobo
	VERSION 0.2
	LANGUAGES CXX
)

set (
	SOURCES

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
	"Window.cpp"
	"WindowManager.cpp"

	"node.cpp"
	"node.hpp"
	"helpers.cpp"
	"helpers.hpp"
)

add_library (${PROJECT_NAME} ${SOURCES})

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
		${IMGUI_INCLUDE_DIRS}
		${GLM_INCLUDE_DIRS}
		"${CMAKE_SOURCE_DIR}/src/external"
)

set_target_properties (
	${PROJECT_NAME}
	PROPERTIES
		CXX_STANDARD 14
		CXX_STANDARD_REQUIRED ON
		CXX_EXTENSIONS OFF
		INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib
		INSTALL_RPATH_USE_LINK_PATH TRUE)

target_link_libraries (${PROJECT_NAME} ${IMGUI_LIBRARY} external_libs glfw assimp ${LUGGCGL_EXTRA_LIBS})

install (TARGETS ${PROJECT_NAME} DESTINATION lib)
