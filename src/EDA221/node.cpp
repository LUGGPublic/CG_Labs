#include "node.hpp"
#include "helpers.hpp"

#include "core/Log.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Node::Node() : _vao(0u), _indices_nb(0u), _program(0u), _textures(), _scaling(1.0f, 1.0f, 1.0f), _rotation(), _translation(), _children()
{
}

void
Node::render(glm::mat4 const& WVP, glm::mat4 const& world) const
{
	if (_vao == 0u || _program == 0u)
		return;

	glUseProgram(_program);

	auto const normal_model_to_world = glm::transpose(glm::inverse(world));

	_set_uniforms(_program);

	glUniformMatrix4fv(glGetUniformLocation(_program, "vertex_model_to_world"), 1, GL_FALSE, glm::value_ptr(world));
	glUniformMatrix4fv(glGetUniformLocation(_program, "normal_model_to_world"), 1, GL_FALSE, glm::value_ptr(normal_model_to_world));
	glUniformMatrix4fv(glGetUniformLocation(_program, "vertex_world_to_clip"), 1, GL_FALSE, glm::value_ptr(WVP));

	glBindVertexArray(_vao);
	glDrawElements(GL_TRIANGLES, _indices_nb, GL_UNSIGNED_INT, reinterpret_cast<GLvoid const*>(0x0));
	glBindVertexArray(0u);

	glUseProgram(0u);
}

void
Node::set_geometry(eda221::mesh_data const& shape)
{
	_vao = shape.vao;
	_indices_nb = static_cast<GLsizei>(shape.indices_nb);
}

void
Node::set_program(GLuint program, std::function<void (GLuint)> const& set_uniforms)
{
	_program = program;
	_set_uniforms = set_uniforms;
}

size_t
Node::get_indices_nb() const
{
	return static_cast<size_t>(_indices_nb);
}

void
Node::set_indices_nb(size_t const& indices_nb)
{
	_indices_nb = static_cast<GLsizei>(indices_nb);
}

void
Node::add_texture(std::string const& name, GLuint tex_id)
{
	if (tex_id != 0u)
		_textures.emplace_back(name, tex_id);
}

void
Node::add_child(Node const* child)
{
	if (child == nullptr)
		LogError("Trying to add a nullptr as child!");
	_children.emplace_back(child);
}

size_t
Node::get_children_nb() const
{
	return _children.size();
}

Node const*
Node::get_child(size_t index) const
{
	assert(index < _children.size());
	return _children[index];
}

void
Node::set_translation(glm::vec3 const& translation)
{
	_translation = translation;
}

void
Node::translate(glm::vec3 const& v)
{
	_translation += v;
}

void
Node::set_scaling(glm::vec3 const& scaling)
{
	_scaling = scaling;
}

void
Node::scale(glm::vec3 const& s)
{
	_scaling *= s;
}

glm::mat4x4
Node::get_transform() const
{
	auto const scaling =  glm::scale(glm::mat4(), _scaling);
	auto const translating = glm::translate(glm::mat4(), _translation);
	auto const rotation_x = glm::rotate(glm::mat4(), _rotation.x, glm::vec3(1.0, 0.0, 0.0));
	auto const rotation_y = glm::rotate(glm::mat4(), _rotation.y, glm::vec3(0.0, 1.0, 0.0));
	auto const rotation_z = glm::rotate(glm::mat4(), _rotation.z, glm::vec3(0.0, 0.0, 1.0));
	auto const rotating = rotation_z * rotation_y * rotation_x;

	//! \todo Use the same code as in assignment 1
	return translating * rotating;
}
