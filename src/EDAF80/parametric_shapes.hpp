#pragma once

#include "core/helpers.hpp"

namespace parametric_shapes
{
	//! \brief Create a quad consisting of two triangles and make it
	//!        available to OpenGL.
	//!
	//! @param width the width of the quad
	//! @param height the height of the quad
	//! @return wrapper around OpenGL objects' name containing the geometry
	//!         data
	bonobo::mesh_data createQuad(unsigned int width, unsigned int height);

	//! \brief Create a sphere for some tesselation level and make it
	//!        available to OpenGL.
	//!
	//! @param res_theta tessellation resolution (nbr of vertices) in the latitude direction ( 0 < theta < PI/2 )
	//! @param res_phi tessellation resolution (nbr of vertices) in the longitude direction ( 0 < phi < 2PI )
	//! @param radius radius of the sphere
	//! @return wrapper around OpenGL objects' name containing the geometry
	//!         data
	bonobo::mesh_data createSphere(unsigned int const res_theta, unsigned int const res_phi, float const radius);

	//! \brief Create a torus for some tesselation level and make it
	//!        available to OpenGL.
	//!
	//! @param res_theta tessellation resolution (nbr of vertices) in the latitude direction ( 0 < theta < 2PI )
	//! @param res_phi tessellation resolution (nbr of vertices) in the longitude direction ( 0 < phi < 2PI )
	//! @param rA radius of the innermost border of the torus
	//! @param rB radius of the outermost border of the torus
	//! @return wrapper around OpenGL objects' name containing the geometry
	//!         data
	bonobo::mesh_data createTorus(unsigned int const res_theta, unsigned int const res_phi, float const rA, float const rB);

	//! \brief Create a circle ring for some tesselation level and make it
	//!        available to OpenGL.
	//!
	//! @param radius_res tessellation resolution (nbr of vertices) in the radial direction ( inner_radius < radius < outer_radius )
	//! @param theta_res tessellation resolution (nbr of vertices) in the angular direction ( 0 < theta < 2PI )
	//! @param inner_radius radius of the innermost border of the ring
	//! @param outer_radius radius of the outermost border of the ring
	//! @return wrapper around OpenGL objects' name containing the geometry
	//!         data
	bonobo::mesh_data createCircleRing(unsigned int const radius_res, unsigned int const theta_res, float const inner_radius, float const outer_radius);
}
