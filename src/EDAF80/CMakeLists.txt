cmake_minimum_required (VERSION 3.0)

set (
	COMMON_SOURCES

	"interpolation.cpp"
	"interpolation.hpp"
	"parametric_shapes.cpp"
	"parametric_shapes.hpp"
)

set (
	ASSIGNMENT1_SOURCES

	"assignment1.cpp"
	"assignment1.hpp"
)

set (
	ASSIGNMENT2_SOURCES

	"assignment2.cpp"
	"assignment2.hpp"
)

source_group (
	EDAF80${PATH_SEP}Assignment2

	FILES
	${PROJECT_SOURCE_DIR}/interpolation.cpp
	${PROJECT_SOURCE_DIR}/interpolation.hpp
	${PROJECT_SOURCE_DIR}/assignment2.cpp
	${PROJECT_SOURCE_DIR}/assignment2.hpp
	${PROJECT_SOURCE_DIR}/parametric_shapes.cpp
	${PROJECT_SOURCE_DIR}/parametric_shapes.hpp
	${SHADERS_DIR}/EDAF80/LambertTexture.vert
	${SHADERS_DIR}/EDAF80/LambertTexture.frag
)

set (
	ASSIGNMENT3_SOURCES

	"assignment3.cpp"
	"assignment3.hpp"
)

source_group (
	EDAF80${PATH_SEP}Assignment3

	FILES
	${PROJECT_SOURCE_DIR}/assignment3.cpp
	${PROJECT_SOURCE_DIR}/assignment3.hpp
	${SHADERS_DIR}/EDAF80/LambertTexture.vert
	${SHADERS_DIR}/EDAF80/LambertTexture.frag
)

set (
	ASSIGNMENT4_SOURCES

	"assignment4.cpp"
	"assignment4.hpp"
)

source_group (
	EDAF80${PATH_SEP}Assignment4

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
	EDAF80${PATH_SEP}Assignment5

	FILES
	${PROJECT_SOURCE_DIR}/assignment5.cpp
	${PROJECT_SOURCE_DIR}/assignment5.hpp
)

luggcgl_new_assignment ("EDAF80_Assignment1" "${ASSIGNMENT1_SOURCES}" "${COMMON_SOURCES}")
luggcgl_new_assignment ("EDAF80_Assignment2" "${ASSIGNMENT2_SOURCES}" "${COMMON_SOURCES}")
luggcgl_new_assignment ("EDAF80_Assignment3" "${ASSIGNMENT3_SOURCES}" "${COMMON_SOURCES}")
luggcgl_new_assignment ("EDAF80_Assignment4" "${ASSIGNMENT4_SOURCES}" "${COMMON_SOURCES}")
luggcgl_new_assignment ("EDAF80_Assignment5" "${ASSIGNMENT5_SOURCES}" "${COMMON_SOURCES}")
