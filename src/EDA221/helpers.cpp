#include "config.hpp"
#include "helpers.hpp"

#include "core/Log.h"
#include "core/Misc.h"
#include "core/opengl.hpp"
#include "core/various.hpp"
#include "external/lodepng.h"

#include <cassert>
#include <memory>
#include <vector>

GLuint
eda221::loadTexture2D(std::string const& filename)
{
	auto const path = config::resources_path("textures/" + filename);
	std::vector<unsigned char> image;
	u32 w, h;
	if (lodepng::decode(image, w, h, path, LCT_RGBA) != 0) {
		LogWarning("Couldn't load or decode image file %s", path.c_str());
		return 0u;
	}
	auto const channels_nb = 4u;
	auto flipBuffer = std::make_unique<u8[]>(w * h * channels_nb);
	for (u32 y = 0; y < h; y++)
		memcpy(flipBuffer.get() + (h - 1 - y) * w * channels_nb, &image[y * w * channels_nb], w * channels_nb);

	GLuint texture = 0u;
	glGenTextures(1, &texture);
	assert(texture != 0u);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, static_cast<GLsizei>(w), static_cast<GLsizei>(h), 0, GL_RGBA, GL_UNSIGNED_BYTE, reinterpret_cast<GLvoid const*>(flipBuffer.get()));
	glBindTexture(GL_TEXTURE_2D, 0u);

	return texture;
}


GLuint
eda221::createProgram(std::string const& vert_shader_source_path, std::string const& frag_shader_source_path)
{
	auto const vertex_shader_source = utils::slurp_file(config::shaders_path("EDA221/" + vert_shader_source_path));
	GLuint vertex_shader = utils::opengl::shader::generate_shader(GL_VERTEX_SHADER, vertex_shader_source);
	assert(vertex_shader != 0u);

	auto const fragment_shader_source = utils::slurp_file(config::shaders_path("EDA221/" + frag_shader_source_path));
	GLuint fragment_shader = utils::opengl::shader::generate_shader(GL_FRAGMENT_SHADER, fragment_shader_source);
	assert(fragment_shader != 0u);

	GLuint program = utils::opengl::shader::generate_program({ vertex_shader, fragment_shader });
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
	assert(program != 0u);
	return program;
}
