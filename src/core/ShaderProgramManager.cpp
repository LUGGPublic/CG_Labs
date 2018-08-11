#include "ShaderProgramManager.hpp"

#include "config.hpp"

#include "Log.h"
#include "opengl.hpp"
#include "various.hpp"

#include <type_traits>

ShaderProgramManager::~ShaderProgramManager()
{
	for (auto const& i : program_entries) {
		if (i.first != 0u) {
			glDeleteProgram(i.first);
			i.first = 0u;
		}
	}
}

void ShaderProgramManager::CreateAndRegisterProgram(ProgramData const& program_data, GLuint& program)
{
	if (!GLAD_GL_ARB_compute_shader) {
		for (auto const& i : program_data) {
			if (i.first == ShaderType::compute) {
				LogError("Compute shaders aren't exposed on your computer (needed for shader '%s'.", i.second.c_str());
				return;
			}
		}
	}

	program_entries.emplace_back(program, program_data);

	ProcessProgram(program_entries.back().second, program_entries.back().first);
}

void ShaderProgramManager::CreateAndRegisterComputeProgram(std::string const& filename, GLuint& program)
{
	if (!GLAD_GL_ARB_compute_shader) {
		LogError("Compute shaders aren't exposed on your computer (needed for shader '%s'.", filename.c_str());
		return;
	}

	program_entries.emplace_back(program, ProgramData{ { ShaderType::compute, filename } });

	ProcessProgram(program_entries.back().second, program_entries.back().first);
}

bool ShaderProgramManager::ReloadAllPrograms()
{
	bool encountered_failures = false;
	for (auto& i : program_entries) {
		if (i.first != 0u)
			glDeleteProgram(i.first);
		i.first = 0u;
		ProcessProgram(i.second, i.first);
		encountered_failures |= i.first == 0u;
	}

	return !encountered_failures;
}

void ShaderProgramManager::ProcessProgram(ProgramData const& program_data, GLuint& program)
{
	std::vector<GLuint> shaders;
	shaders.reserve(program_data.size());

	for (auto const& i : program_data) {
		std::string const full_filename = config::shaders_path(i.second);
		auto const shader_source = utils::slurp_file(full_filename);
		GLuint shader = utils::opengl::shader::generate_shader(static_cast<std::underlying_type<ShaderType>::type>(i.first), shader_source);
		if (shader == 0u) {
			for (auto& shader : shaders)
				glDeleteShader(shader);
			LogError("Compilation of shader '%s' failed; see previous message for details.", full_filename.c_str());
			return;
		}
		shaders.push_back(shader);
	}

	program = utils::opengl::shader::generate_program(shaders);

	for (auto& shader : shaders)
		glDeleteShader(shader);
}
