cmake_minimum_required (VERSION 3.0)

function (luggcgl_new_assignment assignment_name sources common_sources)
	add_executable (${assignment_name} ${sources} ${common_sources})

	target_include_directories (${assignment_name} PRIVATE "${CMAKE_SOURCE_DIR}/src/external")
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

	"node.cpp"
	"node.hpp"
	"helpers.cpp"
	"helpers.hpp"
	"interpolation.cpp"
	"interpolation.hpp"
	"parametric_shapes.cpp"
	"parametric_shapes.hpp"
)

set (
	ASSIGNMENT2_SOURCES

	"assignment2.cpp"
	"assignment2.hpp"
)

source_group (
	EDA221${PATH_SEP}Assignment2

	FILES
	${PROJECT_SOURCE_DIR}/interpolation.cpp
	${PROJECT_SOURCE_DIR}/interpolation.hpp
	${PROJECT_SOURCE_DIR}/assignment2.cpp
	${PROJECT_SOURCE_DIR}/assignment2.hpp
	${PROJECT_SOURCE_DIR}/parametric_shapes.cpp
	${PROJECT_SOURCE_DIR}/parametric_shapes.hpp
	${SHADERS_DIR}/EDA221/LambertTexture.vert
	${SHADERS_DIR}/EDA221/LambertTexture.frag
)

set (
	ASSIGNMENT3_SOURCES

	"assignment3.cpp"
	"assignment3.hpp"
)

source_group (
	EDA221${PATH_SEP}Assignment3

	FILES
	${PROJECT_SOURCE_DIR}/assignment3.cpp
	${PROJECT_SOURCE_DIR}/assignment3.hpp
	${SHADERS_DIR}/EDA221/LambertTexture.vert
	${SHADERS_DIR}/EDA221/LambertTexture.frag
)

set (
	ASSIGNMENT4_SOURCES

	"assignment4.cpp"
	"assignment4.hpp"
)

source_group (
	EDA221${PATH_SEP}Assignment4

	FILES
	${PROJECT_SOURCE_DIR}/assignment4.cpp
	${PROJECT_SOURCE_DIR}/assignment4.hpp
)

set (
	ASSIGNMENT5_SOURCES

	"assignment5.cpp"
	"assignment5.hpp"
)

source_group (
	EDA221${PATH_SEP}Assignment5

	FILES
	${PROJECT_SOURCE_DIR}/assignment5.cpp
	${PROJECT_SOURCE_DIR}/assignment5.hpp
)

luggcgl_new_assignment ("Assignment2" "${ASSIGNMENT2_SOURCES}" "${COMMON_SOURCES}")
luggcgl_new_assignment ("Assignment3" "${ASSIGNMENT3_SOURCES}" "${COMMON_SOURCES}")
luggcgl_new_assignment ("Assignment4" "${ASSIGNMENT4_SOURCES}" "${COMMON_SOURCES}")
luggcgl_new_assignment ("Assignment5" "${ASSIGNMENT5_SOURCES}" "${COMMON_SOURCES}")
