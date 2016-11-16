#include "node.hpp"
#include "helpers.hpp"

#include "core/Log.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Node::Node() : _vao(0u), _vertices_nb(0u), _indices_nb(0u), _drawing_mode(GL_TRIANGLES), _has_indices(true), _program(0u), _textures(), _scaling(1.0f, 1.0f, 1.0f), _rotation(), _translation(), _children()
{
}

void
Node::render(glm::mat4 const& WVP, glm::mat4 const& world) const
{
	render(WVP, world, _program, _set_uniforms);
}

void
Node::render(glm::mat4 const& WVP, glm::mat4 const& world, GLuint program, std::function<void (GLuint)> const& set_uniforms) const
{
	if (_vao == 0u || program == 0u)
		return;

	glUseProgram(program);

	auto const normal_model_to_world = glm::transpose(glm::inverse(world));

	set_uniforms(program);

	glUniformMatrix4fv(glGetUniformLocation(program, "vertex_model_to_world"), 1, GL_FALSE, glm::value_ptr(world));
	glUniformMatrix4fv(glGetUniformLocation(program, "normal_model_to_world"), 1, GL_FALSE, glm::value_ptr(normal_model_to_world));
	glUniformMatrix4fv(glGetUniformLocation(program, "vertex_world_to_clip"), 1, GL_FALSE, glm::value_ptr(WVP));

	glUniform1i(glGetUniformLocation(program, "has_textures"), !_textures.empty());
	bool has_diffuse_texture = false, has_opacity_texture = false;
	for (size_t i = 0u; i < _textures.size(); ++i) {
		auto const texture = _textures[i];
		glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(i));
		glBindTexture(std::get<2>(texture), std::get<1>(texture));
		glUniform1i(glGetUniformLocation(program, std::get<0>(texture).c_str()), static_cast<GLint>(i));
		if (std::get<0>(texture) == "diffuse_texture")
			has_diffuse_texture = true;
		else if (std::get<0>(texture) == "opacity_texture")
			has_opacity_texture = true;
	}
	glUniform1i(glGetUniformLocation(program, "has_diffuse_texture"), has_diffuse_texture);
	glUniform1i(glGetUniformLocation(program, "has_opacity_texture"), has_opacity_texture);

	glBindVertexArray(_vao);
	if (_has_indices)
		glDrawElements(_drawing_mode, _indices_nb, GL_UNSIGNED_INT, reinterpret_cast<GLvoid const*>(0x0));
	else
		glDrawArrays(_drawing_mode, 0, _vertices_nb);
	glBindVertexArray(0u);

	glUseProgram(0u);
}

void
Node::set_geometry(eda221::mesh_data const& shape)
{
	_vao = shape.vao;
	_vertices_nb = static_cast<GLsizei>(shape.vertices_nb);
	_indices_nb = static_cast<GLsizei>(shape.indices_nb);
	_drawing_mode = shape.drawing_mode;
	_has_indices = shape.ibo != 0u;

	if (!shape.bindings.empty()) {
		for (auto const& binding : shape.bindings)
			add_texture(binding.first, binding.second, GL_TEXTURE_2D);
	}
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
Node::add_texture(std::string const& name, GLuint tex_id, GLenum type)
{
	if (tex_id != 0u)
		_textures.emplace_back(name, tex_id, type);
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
