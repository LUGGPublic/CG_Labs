cmake_minimum_required (VERSION 3.0)

function (luggcgl_new_assignment assignment_name sources common_sources)
	add_executable (${assignment_name} ${sources} ${common_sources})

	target_include_directories (${assignment_name} PRIVATE "${CMAKE_SOURCE_DIR}/src/external")
	target_include_directories (${assignment_name} PRIVATE "${CMAKE_SOURCE_DIR}/src/EDA221")
	target_include_directories (${assignment_name} PRIVATE "${CMAKE_SOURCE_DIR}/src")
	target_include_directories (${assignment_name} PRIVATE "${CMAKE_BINARY_DIR}")
	target_include_directories (${assignment_name} PRIVATE ${IMGUI_INCLUDE_DIRS})
	target_include_directories (${assignment_name} PRIVATE ${GLM_INCLUDE_DIRS})
	target_include_directories (${assignment_name} PRIVATE ${ASSIMP_INCLUDE_DIRS})

	set_property (TARGET ${assignment_name} PROPERTY CXX_STANDARD 14)
	set_property (TARGET ${assignment_name} PROPERTY CXX_STANDARD_REQUIRED ON)
	set_property (TARGET ${assignment_name} PROPERTY CXX_EXTENSIONS OFF)

	add_dependencies (${assignment_name} bonobo)

	target_link_libraries (${assignment_name} bonobo ${OPENGL_gl_LIBRARY} ${IMGUI_LIBRARY} ${ASSIMP_LIBRARIES} ${LUGGCGL_EXTRA_LIBS})
endfunction ()

set (
	COMMON_SOURCES

	"../EDA221/node.cpp"
	"../EDA221/node.hpp"
	"../EDA221/helpers.cpp"
	"../EDA221/helpers.hpp"
)

set (
	ASSIGNMENT2_SOURCES

	"assignment2.cpp"
	"assignment2.hpp"
)

source_group (
	EDA221${PATH_SEP}Assignment2

	FILES
	${PROJECT_SOURCE_DIR}/assignment2.cpp
	${PROJECT_SOURCE_DIR}/assignment2.hpp
)

luggcgl_new_assignment ("EDAN35_Assignment2" "${ASSIGNMENT2_SOURCES}" "${COMMON_SOURCES}")
