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
	"Types.cpp"
	"various.cpp"
	"Window.cpp"

	"node.cpp"
	"node.hpp"
	"helpers.cpp"
	"helpers.hpp"
)

add_library (${PROJECT_NAME} ${SOURCES})

target_include_directories (${PROJECT_NAME} PRIVATE ${ASSIMP_INCLUDE_DIRS})
target_include_directories (${PROJECT_NAME} PRIVATE ${IMGUI_INCLUDE_DIRS})
target_include_directories (${PROJECT_NAME} PRIVATE ${GLM_INCLUDE_DIRS})
target_include_directories (${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_include_directories (${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/src/external")
target_include_directories (${PROJECT_NAME} PRIVATE "${CMAKE_BINARY_DIR}")

set_property (TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 14)
set_property (TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
set_property (TARGET ${PROJECT_NAME} PROPERTY CXX_EXTENSIONS OFF)

set_target_properties (${PROJECT_NAME} PROPERTIES
	INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib
	INSTALL_RPATH_USE_LINK_PATH TRUE)

target_link_libraries (${PROJECT_NAME} ${IMGUI_LIBRARY} external_libs glfw assimp ${LUGGCGL_EXTRA_LIBS})

install (TARGETS ${PROJECT_NAME} DESTINATION lib)
