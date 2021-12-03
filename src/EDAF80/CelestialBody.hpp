#pragma once

#include "core/helpers.hpp"
#include "core/node.hpp"

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

struct SpinConfiguration
{
	float axial_tilt{0.0f}; //!< Angle in radians between the body's rotational and orbital axis.
	float speed{0.0f};      //!< Rotation speed in radians per second.
};

struct OrbitConfiguration
{
	float radius{0.0f};      //!< Distance in metres between the centre of the orbit and the body's centre of gravity.
	float inclination{0.0f}; //!< Angle in radians between the body's orbital axis and its parent's rotational axis.
	float speed{0.0f};       //!< Rotation speed in radians per second.
};

//! \brief Represents a celestial body
class CelestialBody
{
public:
	//! \brief Default constructor for a celestial body.
	//!
	//! @param [in] shape Information about the geometry used to model the
	//!             celestial body (more details about it in assignment 2
	//! @param [in] program Shader program used to render the celestial
	//!             body (more details about it in assignment~3)
	//! @param [in] diffuse_texture_id Identifier of the diffuse texture
	//!             used (more details about it also in assignment~3)
	CelestialBody(bonobo::mesh_data const& shape, GLuint const* program,
	              GLuint diffuse_texture_id);

	//! \brief Render this celestial body.
	//!
	//! @param [in] elapsed_time Amount of time (in microseconds) between
	//!             two frames
	//! @param [in] view_projection Matrix transforming from world space to
	//!             clip space
	//! @param [in] parent_transform Matrix transforming from the parent’s
	//!             local space to world space
	//! @param [in] show_basis Show a 3D basis transformed by the world matrix
	//!             of this celestial body
	//! @return Matrix transforming from this celestial body’s local space
	//!         to world space
	glm::mat4 render(std::chrono::microseconds elapsed_time,
	                 glm::mat4 const& view_projection,
	                 glm::mat4 const& parent_transform = glm::mat4(1.0f),
	                 bool show_basis = false);

	//! \brief Mark another celestial body as being “attached” to the current one.
	void add_child(CelestialBody* child);

	//! \brief Return all the children of this celestial body.
	std::vector<CelestialBody*> const& get_children() const;

	//! \brief Configure the orbit parameters for this celestial body.
	void set_orbit(OrbitConfiguration const& configuration);

	//! \brief Configure the scale of this celestial body.
	void set_scale(glm::vec3 const& scale);

	//! \brief Configure the spin parameters for this celestial body.
	void set_spin(SpinConfiguration const& configuration);

	//! \brief Default constructor for a celestial body.
	//!
	//! @param [in] shape Shape used for the rings.
	//! @param [in] program Identifier of the shader program used to render
	//!             the ring
	//! @param [in] diffuse_texture_id Identifier of the diffuse texture
	//!             used by the ring
	//! @param [in] scale How much to scale the ring along the x- and
	//!             y-axis, assuming you are looking at it from above
	void set_ring(bonobo::mesh_data const& shape, GLuint const* program,
	              GLuint diffuse_texture_id,
	              glm::vec2 const& scale = glm::vec2(1.0f));

private:
	struct {
		Node node;
		struct {
			float radius{0.0f};         //!< Distance in metres between its centre of gravity and the centre of the orbit.
			float inclination{0.0f};    //!< Angle in radians between the its orbital axis and its parent's rotational axis.
			float speed{0.0f};          //!< Rotation speed in radians per second.
			float rotation_angle{0.0f}; //!< How much has it rotated around its orbital axis; in radians.
		} orbit;
		glm::vec3 scale{1.0f};
		struct {
			float axial_tilt{0.0f};     //!< Angle in radians between the its rotational and orbital axis.
			float speed{0.0f};          //!< Rotation speed in radians per second.
			float rotation_angle{0.0f}; //!< How much has it rotated around its rotational axis; in radians.
		} spin;
	} _body;

	struct {
		Node node;
		glm::vec2 scale{1.0f};
		bool is_set{false};
	} _ring;

	std::vector<CelestialBody*> _children;
};
