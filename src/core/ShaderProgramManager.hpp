#pragma once

#include "external/glad/glad.h"
#include <GLFW/glfw3.h>

#include <map>
#include <utility>
#include <vector>

#include <cstdint>

enum class ShaderType : std::uint32_t {
	vertex = GL_VERTEX_SHADER,
	tess_eval = GL_TESS_EVALUATION_SHADER,
	tess_ctrl = GL_TESS_CONTROL_SHADER,
	geometry = GL_GEOMETRY_SHADER,
	fragment = GL_FRAGMENT_SHADER,
	compute = GL_COMPUTE_SHADER
};

class ShaderProgramManager
{
public:
	using ProgramData = std::map<ShaderType, std::string>;
	~ShaderProgramManager();
	void CreateAndRegisterProgram(ProgramData const& program_data, GLuint& program);
	void CreateAndRegisterComputeProgram(std::string const& filename, GLuint& program);
	bool ReloadAllPrograms();

private:
	void ProcessProgram(ProgramData const& program_data, GLuint& program);
	using ProgramEntry = std::pair<GLuint&, ProgramData>;
	std::vector<ProgramEntry> program_entries;
};
