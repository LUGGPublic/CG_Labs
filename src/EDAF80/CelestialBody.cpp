#include "CelestialBody.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/trigonometric.hpp>

#include "core/helpers.hpp"
#include "core/Log.h"

CelestialBody::CelestialBody(bonobo::mesh_data const& shape,
                             GLuint const* program,
                             GLuint diffuse_texture_id)
{
    _body.node.set_geometry(shape);
    _body.node.add_texture("diffuse_texture", diffuse_texture_id, GL_TEXTURE_2D);
    _body.node.set_program(program);
}

glm::mat4 CelestialBody::render(std::chrono::microseconds elapsed_time,
                                glm::mat4 const& view_projection,
                                glm::mat4 const& parent_transform,
                                bool show_basis)
{
    // Convert elapsed time from microseconds to seconds
    auto const elapsed_time_s = std::chrono::duration<float>(elapsed_time).count();
    glm::mat4 identity_matrix = glm::mat4(1.0f);
    
    // Ex 1: Scaling
    // Computing the scaling matrix S using GLM's scale() function
    glm::mat4 S = glm::scale(identity_matrix, _body.scale);
    //                             glm::vec3(1.0f, 0.2f, 0.2f));
    
    // Ex 2: Spinning
    // Updating the spin angle based on the elapsed time
    _body.spin.rotation_angle += _body.spin.speed * elapsed_time_s;
    
    // Computing the first rotation matrix (R1,s) - Spin around y-axis
    glm::mat4 R1_s = glm::rotate(identity_matrix, _body.spin.rotation_angle, glm::vec3(0.0f, 1.0f, 0.0f));
    // Computing the second rotation matrix (R2,s) - Tilt around z-axis (axial tilt)
    glm::mat4 R2_s = glm::rotate(identity_matrix, _body.spin.axial_tilt, glm::vec3(0.0f, 0.0f, 1.0f));
    
    // Ex 3: Adding Orbits
    // Computing the translation matrix T_0 to move the celestial body to its orbit
    // Moving along the x-axis by the orbit radius
    glm::mat4 T_0 = glm::translate(identity_matrix, glm::vec3(_body.orbit.radius, 0.0f, 0.0f));
    // Updating the orbit angle based on the elapsed time (for motion in the x-z plane)
    _body.orbit.rotation_angle += _body.orbit.speed * elapsed_time_s;
    // Computing the orbit rotation matrix (R_0), rotating around the y-axis
    glm::mat4 R_0 = glm::rotate(identity_matrix, _body.orbit.rotation_angle, glm::vec3(0.0f, 1.0f, 0.0f));
    // Computing the matrix tilting the orbit plane around the z-axis (R2_0)
    glm::mat4 R2_0 = glm::rotate(identity_matrix, _body.orbit.inclination, glm::vec3(0.0f, 0.0f, 1.0f));
    
    // Combine the matrices in the correct order (including parent_transform)
    // Order: Parent transform -> Tilt orbit plane (R2_0) -> Orbit rotation (R_0) -> Translation to orbit (T_0)
    // -> Spin (R1_s) -> Tilt (R2_s) -> Scaling (S)
    glm::mat4 world = parent_transform * R2_0 * R_0 * T_0 * R1_s * R2_s * S;
    
    // Optionally show the coordinate basis if the flag is set
    if (show_basis)
    {
        bonobo::renderBasis(1.0f, 2.0f, view_projection, world);
    }
    
    // Render the celestial body with the updated world matrix
    _body.node.render(view_projection, world);
    
    // Compute the matrix to pass to the children
    // Children will inherit the parent transform, orbit, and tilt (ignoring scale and spin)
    glm::mat4 child_transform = parent_transform * R2_0 * R_0 * T_0 * R2_s;
    
    // Return the transformation to be used by the children
    return child_transform;
    
}

void CelestialBody::add_child(CelestialBody* child)
{
    _children.push_back(child);
}

std::vector<CelestialBody*> const& CelestialBody::get_children() const
{
    return _children;
}

void CelestialBody::set_orbit(OrbitConfiguration const& configuration)
{
    _body.orbit.radius = configuration.radius;
    _body.orbit.inclination = configuration.inclination;
    _body.orbit.speed = configuration.speed;
    _body.orbit.rotation_angle = 0.0f;
}

void CelestialBody::set_scale(glm::vec3 const& scale)
{
    _body.scale = scale;
}

void CelestialBody::set_spin(SpinConfiguration const& configuration)
{
    _body.spin.axial_tilt = configuration.axial_tilt;
    _body.spin.speed = configuration.speed;
    _body.spin.rotation_angle = 0.0f;
}

void CelestialBody::set_ring(bonobo::mesh_data const& shape,
                             GLuint const* program,
                             GLuint diffuse_texture_id,
                             glm::vec2 const& scale)
{
    _ring.node.set_geometry(shape);
    _ring.node.add_texture("diffuse_texture", diffuse_texture_id, GL_TEXTURE_2D);
    _ring.node.set_program(program);
    
    _ring.scale = scale;
    
    _ring.is_set = true;
}
