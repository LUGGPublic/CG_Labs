#pragma once

#include "external/glad/glad.h"
#include <GLFW/glfw3.h>

#include <string>
#include <vector>

//! \brief Namespace containing a few helpers for the EDA221 labs.
namespace eda221
{
	//! \brief Formalise mapping between an OpenGL VAO attribute binding,
	//!        and the meaning of that attribute.
	enum class shader_bindings : unsigned int{
		vertices = 0u, //!< = 0, value of the binding point for vertices
		normals,       //!< = 1, value of the binding point for normals
		texcoords,     //!< = 2, value of the binding point for texcoords
		tangents,      //!< = 3, value of the binding point for tangents
		binormals      //!< = 4, value of the binding point for binormals
	};

	//! \brief Contains the data for a mesh in OpenGL.
	struct mesh_data {
		GLuint vao;        //!< OpenGL name of the Vertex Array Object
		GLuint bo;         //!< OpenGL name of the Buffer Object
		GLuint ibo;        //!< OpenGL name of the Buffer Object for indices
		size_t indices_nb; //!< number of indices stored in ibo
	};

	//! \brief Load objects found in an object/scene file, using assimp.
	//!
	//! @param [in] filename of the object/scene file to load, relative to
	//!             the `res/scenes` folder
	//! @return a vector of filled in `mesh_data` structures, one per
	//!         object found in the input file
	std::vector<mesh_data> loadObjects(std::string const& filename);

	//! \brief Load a PNG image into an OpenGL 2D-texture.
	//!
	//! @param [in] filename of the PNG image, relative to the `textures`
	//!             folder within the `resources` folder.
	//! @return the name of the OpenGL 2D-texture
	GLuint loadTexture2D(std::string const& filename);

	//! \brief Load six PNG images into an OpenGL cubemap-texture.
	//!
	//! @param [in] posx path to the texture on the left of the cubemap
	//! @param [in] negx path to the texture on the right of the cubemap
	//! @param [in] posy path to the texture on the top of the cubemap
	//! @param [in] negy path to the texture on the bottom of the cubemap
	//! @param [in] negz path to the texture on the front of the cubemap
	//! @param [in] posz path to the texture on the back of the cubemap
	//! @return the name of the OpenGL cubemap-texture
	//!
	//! All paths are relative to the `res/cubemaps` folder.
	GLuint loadTextureCubeMap(std::string const& posx, std::string const& negx,
                                  std::string const& posy, std::string const& negy,
                                  std::string const& negz, std::string const& posz,
                                  bool generate_mipmap = false);

	//! \brief Create an OpenGL program consisting of a vertex and a
	//!        fragment shader.
	//!
	//! @param [in] vert_shader_source_path of the vertex shader source
	//!             code, relative to the `shaders/EDA221` folder
	//! @param [in] frag_shader_source_path of the fragment shader source
	//!             code, relative to the `shaders/EDA221` folder
	//! @return the name of the OpenGL shader program
	GLuint createProgram(std::string const& vert_shader_source_path,
	                     std::string const& frag_shader_source_path);
}
