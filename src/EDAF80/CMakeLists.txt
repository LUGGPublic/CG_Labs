add_library (interpolation STATIC)
target_sources (
       interpolation
       PUBLIC [[interpolation.hpp]]
       PRIVATE [[interpolation.cpp]]
)
target_link_libraries (interpolation PRIVATE CG_Labs_options glm)

add_library (parametric_shapes STATIC)
target_sources (
       parametric_shapes
       PUBLIC [[parametric_shapes.hpp]]
       PRIVATE [[parametric_shapes.cpp]]
)
target_link_libraries (parametric_shapes PRIVATE bonobo CG_Labs_options)


# Assignment 1
add_executable (EDAF80_Assignment1)
target_sources (
	EDAF80_Assignment1
	PRIVATE
		[[assignment1.cpp]]
		[[CelestialBody.cpp]]
		[[CelestialBody.hpp]]
)
target_link_libraries (
	EDAF80_Assignment1
	PRIVATE assignment_setup parametric_shapes
)
copy_dlls (EDAF80_Assignment1 "${CMAKE_CURRENT_BINARY_DIR}")


# Assignment 2
add_executable (EDAF80_Assignment2)
target_sources (
	EDAF80_Assignment2
	PRIVATE
		[[assignment2.hpp]]
		[[assignment2.cpp]]
)
target_link_libraries (
	EDAF80_Assignment2
	PRIVATE assignment_setup interpolation parametric_shapes
)
copy_dlls (EDAF80_Assignment2 "${CMAKE_CURRENT_BINARY_DIR}")


# Assignment 3
add_executable (EDAF80_Assignment3)
target_sources (
	EDAF80_Assignment3
	PRIVATE
		[[assignment3.hpp]]
		[[assignment3.cpp]]
)
target_link_libraries (
	EDAF80_Assignment3
	PRIVATE assignment_setup parametric_shapes
)
copy_dlls (EDAF80_Assignment3 "${CMAKE_CURRENT_BINARY_DIR}")


# Assignment 4
add_executable (EDAF80_Assignment4)
target_sources (
	EDAF80_Assignment4
	PRIVATE
		[[assignment4.hpp]]
		[[assignment4.cpp]]
)
target_link_libraries (
	EDAF80_Assignment4
	PRIVATE assignment_setup parametric_shapes
)
copy_dlls (EDAF80_Assignment4 "${CMAKE_CURRENT_BINARY_DIR}")


# Assignment 5
add_executable (EDAF80_Assignment5)
target_sources (
	EDAF80_Assignment5
	PRIVATE
		[[assignment5.hpp]]
		[[assignment5.cpp]]
)
target_link_libraries (EDAF80_Assignment5 PRIVATE assignment_setup)
copy_dlls (EDAF80_Assignment5 "${CMAKE_CURRENT_BINARY_DIR}")


install (
	TARGETS
		EDAF80_Assignment1
		EDAF80_Assignment2
		EDAF80_Assignment3
		EDAF80_Assignment4
		EDAF80_Assignment5
	DESTINATION [[bin]]
)
