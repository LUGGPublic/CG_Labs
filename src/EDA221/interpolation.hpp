#pragma once

#include <glm/glm.hpp>

namespace interpolation
{
	//! \brief Linearly interpolate a position between two points.
	//!
	//! @param [in] p0 origin point for the interpolation
	//! @param [in] p1 destination point for the interpolation
	//! @param [in] x distance ratio between p0 and p1 at which the
	//!               interpolated point should be:
	//!               * x == 0.0: result will be p0;
	//!               * x == 1.0: result will be p1;
	//!               * x == 0.5: result will be the midpoint of [p0,p1]
	//!               * x ∈ ]0,1[: result will be somewhere on ]p0,p1[
	//! @return interpolated position
	glm::vec3 evalLERP(glm::vec3 const&p0, glm::vec3 const&p1,
	                   float const x);

	//! \brief Compute a new position using a Catmull-Rom spline
	//!        interpolation.
	//!
	//! @param [in] p0 \f$p[i-1]\f$
	//! @param [in] p1 \f$p[i]\f$
	//! @param [in] p2 \f$p[i+1]\f$
	//! @param [in] p3 \f$p[i+2]\f$
	//! @param [in] t tension
	//! @param [in] x distance ratio between p1 and p2 at which the
	//!               interpolated point should be:
	//!               * x == 0.0: result will be p0;
	//!               * x == 1.0: result will be p1;
	//!               * x == 0.5: result will be the midpoint of [p0,p1]
	//!               * x ∈ ]0,1[: result will be somewhere on ]p0,p1[
	//! @return interpolated position
	glm::vec3 evalCatmullRom(glm::vec3 const&p0, glm::vec3 const&p1,
	                         glm::vec3 const&p2, glm::vec3 const&p3,
	                         float const t, float const x);
}
