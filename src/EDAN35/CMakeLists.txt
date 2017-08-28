cmake_minimum_required (VERSION 3.0)

set (
	ASSIGNMENT2_SOURCES

	"assignment2.cpp"
	"assignment2.hpp"
)

source_group (
	EDAN35${PATH_SEP}Assignment2

	FILES
	${PROJECT_SOURCE_DIR}/assignment2.cpp
	${PROJECT_SOURCE_DIR}/assignment2.hpp
)

luggcgl_new_assignment ("EDAN35_Assignment2" "${ASSIGNMENT2_SOURCES}" "")
