#include "node.hpp"
#include "helpers.hpp"

#include "core/Log.h"
#include "core/opengl.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Node::Node() : _vao(0u), _vertices_nb(0u), _indices_nb(0u), _drawing_mode(GL_TRIANGLES), _has_indices(true), _program(nullptr), _textures(), _transform(), _children()
{
}

void
Node::render(glm::mat4 const& WVP, glm::mat4 const& parentTransform) const
{
	if (_program != nullptr)
		render(WVP, parentTransform * _transform.GetMatrix(), *_program, _set_uniforms);
}

void
Node::render(glm::mat4 const& WVP, glm::mat4 const& world, GLuint program, std::function<void (GLuint)> const& set_uniforms) const
{
	if (_vao == 0u || program == 0u)
		return;

	if (utils::opengl::debug::isSupported())
	{
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0u, _name.size(), _name.data());
	}

	glUseProgram(program);

	auto const normal_model_to_world = glm::transpose(glm::inverse(world));

	set_uniforms(program);

	glUniformMatrix4fv(glGetUniformLocation(program, "vertex_model_to_world"), 1, GL_FALSE, glm::value_ptr(world));
	glUniformMatrix4fv(glGetUniformLocation(program, "normal_model_to_world"), 1, GL_FALSE, glm::value_ptr(normal_model_to_world));
	glUniformMatrix4fv(glGetUniformLocation(program, "vertex_world_to_clip"), 1, GL_FALSE, glm::value_ptr(WVP));

	for (size_t i = 0u; i < _textures.size(); ++i) {
		auto const& texture = _textures[i];
		glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(i));
		glBindTexture(std::get<2>(texture), std::get<1>(texture));
		glUniform1i(glGetUniformLocation(program, std::get<0>(texture).c_str()), static_cast<GLint>(i));

		std::string texture_presence_var_name = "has_" + std::get<0>(texture);
		glUniform1i(glGetUniformLocation(program, texture_presence_var_name.c_str()), 1);
	}

	glBindVertexArray(_vao);
	if (_has_indices)
		glDrawElements(_drawing_mode, _indices_nb, GL_UNSIGNED_INT, reinterpret_cast<GLvoid const*>(0x0));
	else
		glDrawArrays(_drawing_mode, 0, _vertices_nb);
	glBindVertexArray(0u);

	for (auto const& texture : _textures) {
		glBindTexture(std::get<2>(texture), 0);
		glUniform1i(glGetUniformLocation(program, std::get<0>(texture).c_str()), 0);

		std::string texture_presence_var_name = "has_" + std::get<0>(texture);
		glUniform1i(glGetUniformLocation(program, texture_presence_var_name.c_str()), 0);
	}

	glUseProgram(0u);

	if (utils::opengl::debug::isSupported())
	{
		glPopDebugGroup();
	}
}

void
Node::set_geometry(bonobo::mesh_data const& shape)
{
	_vao = shape.vao;
	_vertices_nb = static_cast<GLsizei>(shape.vertices_nb);
	_indices_nb = static_cast<GLsizei>(shape.indices_nb);
	_drawing_mode = shape.drawing_mode;
	_has_indices = shape.ibo != 0u;
	_name = shape.name;

	if (!shape.bindings.empty()) {
		for (auto const& binding : shape.bindings)
			add_texture(binding.first, binding.second, GL_TEXTURE_2D);
	}
}

void
Node::set_program(GLuint const* const program, std::function<void (GLuint)> const& set_uniforms)
{
	if (program == nullptr)
		throw std::runtime_error("Node::set_program: program can not be null.");

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

TRSTransformf const&
Node::get_transform() const
{
	return _transform;
}

TRSTransformf&
Node::get_transform()
{
	return _transform;
}
