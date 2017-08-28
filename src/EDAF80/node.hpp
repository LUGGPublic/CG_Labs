#pragma once

#include "external/glad/glad.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <functional>
#include <tuple>
#include <vector>

namespace bonobo
{
	struct mesh_data;
}

//! \brief Represents a node of a scene graph
class Node
{
public:
	//! \brief Default constructor.
	Node();

	//! \brief Render this node.
	//!
	//! @param [in] WVP Matrix transforming from world-space to clip-space
	//! @param [in] world Matrix transforming from model-space to
	//!             world-space
	void render(glm::mat4 const& WVP, glm::mat4 const& world) const;

	//! \brief Render this node with a specific shader program.
	//!
	//! @param [in] WVP Matrix transforming from world-space to clip-space
	//! @param [in] world Matrix transforming from model-space to
	//!             world-space
	//! @param [in] program OpenGL shader program to use
	//! @param [in] set_uniforms function that will take as argument an
	//!             OpenGL shader program, and will setup that program's
	//!             uniforms
	void render(glm::mat4 const& WVP, glm::mat4 const& world,
	            GLuint program,
	            std::function<void (GLuint)> const& set_uniforms) const;

	//! \brief Set the geometry of this node.
	//!
	//! A node without any geometry will not render itself, but its
	//! children will be rendered if they have any geometry.
	//!
	//! @param [in] shape OpenGL data to use as geometry
	void set_geometry(bonobo::mesh_data const& shape);

	//! \brief Get the number of indices to use.
	//!
	//! @return how many indices to use when rendering
	size_t get_indices_nb() const;

	//! \brief Set the number of indices to use.
	//!
	//! @param [in] indices_nb how many indices to use when rendering
	void set_indices_nb(size_t const& indices_nb);

	//! \brief Set the program of this node.
	//!
	//! A node without a program will not render itself, but its children
	//! will be rendered if they have one.
	//!
	//! @param [in] program OpenGL shader program to use
	//! @param [in] set_uniforms function that will take as argument an
	//!             OpenGL shader program, and will setup that program's
	//!             uniforms
	void set_program(GLuint program, std::function<void (GLuint)> const& set_uniforms);

	//! \brief Add a texture to this node.
	//!
	//! @param [in] name the variable name used by the attached OpenGL
	//!                  shader program; in assignment 1, this will be
	//!                  `diffuse_texture`
	//! @param [in] tex_id the name of an OpenGL 2D-texture
	//! @param [in] type the type of texture; defaults to GL_TEXTURE_2D
	void add_texture(std::string const& name, GLuint tex_id, GLenum type);

	//! \brief Add a child to this node.
	//!
	//! @param [in] child pointer to the child to add; the pointer has to
	//!             be non-null
	void add_child(Node const* child);

	//! \brief Return the number of children to this node.
	//!
	//! @return the number of children
	size_t get_children_nb() const;

	//! \brief Return the ith child.
	//!
	//! @param [in] index the index of the child to return; index should be
	//!             strictly less than the number of children
	//! @return a pointer to the desired child
	Node const* get_child(size_t index) const;

	//! \brief Reset the translation to a new value.
	//!
	//! @param [in] translation new translation vector
	void set_translation(glm::vec3 const& translation);

	//! \brief Translate this node.
	//!
	//! @param [in] v translation vector to be added to the node's
	//!               current translation
	void translate(glm::vec3 const& v);

	//! \brief Reset the rotation along the x-axis to a new value.
	//!
	//! @param [in] angle new rotation angle along the x-axis; it should be
	//!                   given in radians
	void set_rotation_x(float angle) { _rotation.x = angle; }

	//! \brief Rotate this node along the x-axis.
	//!
	//! @param [in] d_angle delta angle to add to the current rotation
	//!                     angle around the x-axis; it should be given in
	//!                     radians
	void rotate_x(float d_angle) { _rotation.x += d_angle; }

	//! \brief Reset the rotation along the y-axis to a new value.
	//!
	//! @param [in] angle new rotation angle along the y-axis; it should be
	//!                   given in radians
	void set_rotation_y(float angle) { _rotation.y = angle; }

	//! \brief Rotate this node along the y-axis.
	//!
	//! @param [in] d_angle delta angle to add to the current rotation
	//!                     angle around the y-axis; it should be given in
	//!                     radians
	void rotate_y(float d_angle) { _rotation.y += d_angle; }

	//! \brief Reset the rotation along the z-axis to a new value.
	//!
	//! @param [in] angle new rotation angle along the z-axis; it should be
	//!                   given in radians
	void set_rotation_z(float angle) { _rotation.z = angle; }

	//! \brief Rotate this node along the z-axis.
	//!
	//! @param [in] d_angle delta angle to add to the current rotation
	//!                     angle around the z-axis; it should be given in
	//!                     radians
	void rotate_z(float d_angle) { _rotation.z += d_angle; }

	//! \brief Reset the scaling to a new value.
	//!
	//! @param [in] scaling new scaling vector: `(x_scaling, y_scaling,
	//!                     z_scaling)`
	void set_scaling(glm::vec3 const& scaling);

	//! \brief Scale this node.
	//!
	//! @param [in] s scaling vector to be composed with the node's
	//!               current scaling value
	void scale(glm::vec3 const& s);

	//! \brief Return this node transformation matrix.
	//!
	//! @return the composition of the rotation, scaling and translation
	//!         transformations; this is the model matrix of this node
	glm::mat4x4 get_transform() const;

private:
	// Geometry data
	GLuint _vao;
	GLsizei _vertices_nb;
	GLsizei _indices_nb;
	GLenum _drawing_mode;
	bool _has_indices;

	// Program data
	GLuint _program;
	std::function<void (GLuint)> _set_uniforms;

	// Textures data
	std::vector<std::tuple<std::string, GLuint, GLenum>> _textures;

	// Transformation data
	glm::vec3 _scaling;
	glm::vec3 _rotation; // as (angle around x-axis, angle around y-axis, angle around z-axis)
	glm::vec3 _translation;

	// Children data
	std::vector<Node const*> _children;
};
