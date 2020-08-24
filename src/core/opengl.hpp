#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>
#include <vector>


namespace utils
{

namespace opengl
{

namespace debug
{

bool isSupported();
std::string getStringForType(GLenum type);
std::string getStringForSource(GLenum source);
std::string getStringForSeverity(GLenum severity);
void
#ifdef _WIN32
APIENTRY
#endif
opengl_error_callback( GLenum source, GLenum type, GLuint id
                     , GLenum severity, GLsizei /*length*/
                     , GLchar const* msg, void const* /*data*/
                     );

} // end of namespace debug

namespace shader
{

bool source_and_build_shader(GLuint id, std::string const& source);
GLuint generate_shader(GLenum type, std::string const& source);
bool link_program(GLuint id);
void reload_program(GLuint id, std::vector<GLuint> const& ids, std::vector<std::string> const& sources);
GLuint generate_program(std::vector<GLuint> const& shaders_id);

} // end of namespace shader

namespace fullscreen
{

void init(std::string const& vs_path, std::string const& fs_path, size_t width, size_t height);
void deinit();
void draw();
GLuint get_texture_id();

} // end of namespace fullscreen

} // end of namespace opengl

} // end of namespace utils
