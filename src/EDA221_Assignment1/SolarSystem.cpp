#include "SolarSystem.h"

#include "config.hpp"
#include "external/glad/glad.h"
#include "core/FPSCamera.h"
#include "core/InputHandler.h"
#include "core/Log.h"
#include "core/LogView.h"
#include "core/Misc.h"
#include "core/opengl.hpp"
#include "core/utils.h"
#include "core/various.hpp"
#include "core/Window.h"
#include <imgui.h>
#include "external/imgui_impl_glfw_gl3.h"
#include "external/lodepng.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <array>
#include <chrono>
#include <cstdlib>
#include <unordered_map>
#include <stack>
#include <stdexcept>
#include <string>
#include <vector>


enum class shader_bindings {
	vertices = 0u,
	normals,
	texcoords
};

struct sphere_t {
	GLuint vao;
	GLuint vbo;
	GLuint nbo;
	GLuint tbo;
	GLuint ibo;
	GLsizei indices_nb;

	sphere_t() : vao(0u), vbo(0u), nbo(0u), tbo(0u), ibo(0u), indices_nb(0u) {
	}

	sphere_t(sphere_t&& other) : vao(other.vao), vbo(other.vbo), nbo(other.nbo), tbo(other.tbo), ibo(other.ibo), indices_nb(other.indices_nb) {
		other.vao = 0u;
		other.vbo = 0u;
		other.nbo = 0u;
		other.tbo = 0u;
		other.ibo = 0u;
		other.indices_nb = 0u;
	}

	~sphere_t() {
		glDeleteBuffers(1, &ibo);
		ibo = 0u;

		glDeleteBuffers(1, &tbo);
		tbo = 0u;

		glDeleteBuffers(1, &nbo);
		nbo = 0u;

		glDeleteBuffers(1, &vbo);
		vbo = 0u;

		glDeleteVertexArrays(1, &vao);
		vao = 0u;
	}
};

static sphere_t loadSphere();
static GLuint loadTexture2D(std::string const& filename);
static GLuint createProgram(std::string const& vert_shader_source_path, std::string const& frag_shader_source_path);


class Node
{
public:
	Node();

	void render(glm::mat4 const& WVP, glm::mat4 const& world) const;

	void set_geometry(sphere_t const& sphere);
	void set_program(GLuint program);
	void add_texture(std::string const& name, GLuint tex_id);
	void add_child(Node const* child);
	size_t get_children_nb() const;
	Node const* get_child(size_t index) const;

	void set_translation(glm::vec3 const& translation);
	void set_rotation_x(float angle) { _rotation.x = angle; }
	void set_rotation_y(float angle) { _rotation.y = angle; }
	void set_rotation_z(float angle) { _rotation.z = angle; }
	void rotate_x(float angle) { _rotation.x += angle; }
	void rotate_y(float angle) { _rotation.y += angle; }
	void rotate_z(float angle) { _rotation.z += angle; }
	void set_scaling(glm::vec3 const& scaling);
	glm::mat4x4 get_transform() const;

private:
	// Geometry data
	GLuint _vao;
	GLsizei _indices_nb;

	// Program data
	GLuint _program;

	// Textures data
	std::vector<std::pair<std::string, GLuint>> _textures;

	// Transformation data
	glm::vec3 _scaling;
	glm::vec3 _rotation;
	glm::vec3 _translation;

	// Children data
	std::vector<Node const*> _children;
};

Node::Node() : _vao(0u), _indices_nb(0u), _program(0u), _textures(), _scaling(1.0f, 1.0f, 1.0f), _rotation(), _translation(), _children()
{
}

void
Node::render(glm::mat4 const& WVP, glm::mat4 const& world) const
{
	if (_vao == 0u || _program == 0u)
		return;

	glUseProgram(_program);

	glUniformMatrix4fv(glGetUniformLocation(_program, "vertex_model_to_world"), 1, GL_FALSE, glm::value_ptr(world));
	glUniformMatrix4fv(glGetUniformLocation(_program, "vertex_world_to_clip"), 1, GL_FALSE, glm::value_ptr(WVP));

	glUniform1i(glGetUniformLocation(_program, "has_textures"), static_cast<int>(!_textures.empty()));

	for (size_t i = 0u; i < _textures.size(); ++i) {
		glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(i));
		glBindTexture(GL_TEXTURE_2D, _textures[i].second);
		glUniform1i(glGetUniformLocation(_program, _textures[i].first.c_str()), static_cast<GLint>(i));
	}

	glBindVertexArray(_vao);
	glDrawElements(GL_TRIANGLES, _indices_nb, GL_UNSIGNED_INT, reinterpret_cast<GLvoid const*>(0x0));
	glBindVertexArray(0u);

	glUseProgram(0u);
}

void
Node::set_geometry(sphere_t const& sphere)
{
	_vao = sphere.vao;
	_indices_nb = sphere.indices_nb;
}

void
Node::set_program(GLuint program)
{
	_program = program;
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
Node::set_scaling(glm::vec3 const& scaling)
{
	_scaling = scaling;
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

	//
	// Todo: Compute the node's object matrix
	//
	return translating;
}

GLuint
loadTexture2D(std::string const& filename)
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
createProgram(std::string const& vert_shader_source_path, std::string const& frag_shader_source_path)
{
	auto const vertex_shader_source = utils::slurp_file(config::shaders_path(vert_shader_source_path));
	GLuint vertex_shader = utils::opengl::shader::generate_shader(GL_VERTEX_SHADER, vertex_shader_source);
	assert(vertex_shader != 0u);

	auto const fragment_shader_source = utils::slurp_file(config::shaders_path(frag_shader_source_path));
	GLuint fragment_shader = utils::opengl::shader::generate_shader(GL_FRAGMENT_SHADER, fragment_shader_source);
	assert(fragment_shader != 0u);

	GLuint program = utils::opengl::shader::generate_program({ vertex_shader, fragment_shader });
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
	assert(program != 0u);
	return program;
}


SolarSystem::SolarSystem()
{
	Log::View::Init();

	window = Window::Create("EDA221: Assignment 1", config::resolution_x,
	                        config::resolution_y, config::msaa_rate, false);
	if (window == nullptr) {
		Log::View::Destroy();
		throw std::runtime_error("Failed to get a window: aborting!");
	}
	inputHandler = new InputHandler();
	window->SetInputHandler(inputHandler);

	glEnable(GL_DEPTH_TEST);
}

SolarSystem::~SolarSystem()
{
	delete inputHandler;
	inputHandler = nullptr;

	Window::Destroy(window);
	window = nullptr;

	Log::View::Destroy();
}

void SolarSystem::run()
{
	// Load the sphere geometry
	auto const sphere = loadSphere();
	if (sphere.vao == 0u)
		return;

	// Set up the camera
	FPSCameraf mCamera(bonobo::pi / 4.0f,
	                   static_cast<float>(config::resolution_x) / static_cast<float>(config::resolution_y),
	                   0.01f, 1000.0f);
	mCamera.mWorld.SetTranslate(glm::vec3(0.0f, 0.0f, 6.0f));
	mCamera.mMouseSensitivity = 0.003f;
	mCamera.mMovementSpeed = 0.25f * 12.0f;
	window->SetCamera(&mCamera);

	// Create the shader program
	auto shader = createProgram("EDA221_Assignment1/default.vert", "EDA221_Assignment1/default.frag");
	if (shader == 0u) {
		LogError("Failed to load shader");
		return;
	}

	// Load the sun's texture
	auto sun_texture = loadTexture2D("sunmap.png");

	auto sun = Node();
	sun.set_geometry(sphere);
	sun.set_program(shader);
	//
	// Todo: Attach a texture to the sun
	//


	auto world = Node();
	world.add_child(&sun);


	//
	// Todo: Create an Earth node
	//


	f64 ddeltatime;
	size_t fpsSamples = 0;
	double nowTime, lastTime = GetTimeSeconds();
	double fpsNextTick = lastTime + 1.0;

	while (!glfwWindowShouldClose(window->GetGLFW_Window())) {
		nowTime = GetTimeSeconds();
		ddeltatime = nowTime - lastTime;
		if (nowTime > fpsNextTick) {
			fpsNextTick += 1.0;
			fpsSamples = 0;
		}
		fpsSamples++;

		glfwPollEvents();
		inputHandler->Advance();
		mCamera.Update(ddeltatime, *inputHandler);

		ImGui_ImplGlfwGL3_NewFrame();


		//
		// How-To: Translate the sun
		//
		sun.set_translation(glm::vec3(std::sin(nowTime), 0.0f, 0.0f));


		auto const window_size = window->GetDimensions();
		glViewport(0, 0, window_size.x, window_size.y);
		glClearDepthf(1.0f);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		// Traverse the scene graph and render all the nodes
		auto node_stack = std::stack<Node const*>();
		auto matrix_stack = std::stack<glm::mat4>();
		node_stack.push(&world);
		matrix_stack.push(glm::mat4());
		do {
			auto const* const current_node = node_stack.top();
			node_stack.pop();

			auto const parent_matrix = matrix_stack.top();
			matrix_stack.pop();

			auto const current_node_matrix = current_node->get_transform();

			//
			// Todo: Compute the current node's world matrix
			//
			auto const current_node_world_matrix = current_node_matrix;
			current_node->render(mCamera.GetWorldToClipMatrix(), current_node_world_matrix);

			for (int i = static_cast<int>(current_node->get_children_nb()) - 1; i >= 0; --i) {
				node_stack.push(current_node->get_child(static_cast<size_t>(i)));
				matrix_stack.push(current_node_world_matrix);
			}
		} while (!node_stack.empty());

		Log::View::Render();
		ImGui::Render();

		window->Swap();
		lastTime = nowTime;
	}

	glDeleteProgram(shader);
	shader = 0u;
}


sphere_t
loadSphere()
{
	sphere_t sphere;
	sphere.vao = 0u;

	auto const scene_filepath = config::resources_path("scenes/sphere.obj");
	Assimp::Importer importer;
	auto const assimp_scene = importer.ReadFile(scene_filepath, 0u);
	if (assimp_scene == nullptr) {
		LogError("Assimp failed to load \"%s\": %s", scene_filepath.c_str(), importer.GetErrorString());
		return sphere;
	}

	if (assimp_scene->mNumMeshes == 0u) {
		LogError("No mesh available; loading \"%s\" must have had issues", scene_filepath.c_str());
		return sphere;
	}
	auto const assimp_sphere_mesh = assimp_scene->mMeshes[0u];

	glGenVertexArrays(1, &sphere.vao);
	assert(sphere.vao != 0u);
	glBindVertexArray(sphere.vao);

	sphere.vbo = 0u;
	glGenBuffers(1, &sphere.vbo);
	assert(sphere.vbo != 0u);
	glBindBuffer(GL_ARRAY_BUFFER, sphere.vbo);
	glBufferData(GL_ARRAY_BUFFER, (assimp_sphere_mesh->mNumVertices * 3u) * sizeof(GLfloat), reinterpret_cast<GLvoid const*>(assimp_sphere_mesh->mVertices), GL_STATIC_DRAW);
	glEnableVertexAttribArray(static_cast<int>(shader_bindings::vertices));
	glVertexAttribPointer(static_cast<int>(shader_bindings::vertices), 3, GL_FLOAT, false, 0, reinterpret_cast<GLvoid const*>(0x0));

	sphere.nbo = 0u;
	glGenBuffers(1, &sphere.nbo);
	assert(sphere.nbo != 0u);
	glBindBuffer(GL_ARRAY_BUFFER, sphere.nbo);
	glBufferData(GL_ARRAY_BUFFER, (assimp_sphere_mesh->mNumVertices * 3u) * sizeof(GLfloat), reinterpret_cast<GLvoid const*>(assimp_sphere_mesh->mNormals), GL_STATIC_DRAW);
	glEnableVertexAttribArray(static_cast<int>(shader_bindings::normals));
	glVertexAttribPointer(static_cast<int>(shader_bindings::normals), 3, GL_FLOAT, false, 0, reinterpret_cast<GLvoid const*>(0x0));

	sphere.tbo = 0u;
	glGenBuffers(1, &sphere.tbo);
	assert(sphere.tbo != 0u);
	glBindBuffer(GL_ARRAY_BUFFER, sphere.tbo);
	glBufferData(GL_ARRAY_BUFFER, (assimp_sphere_mesh->mNumVertices * 3u) * sizeof(GLfloat), reinterpret_cast<GLvoid const*>(assimp_sphere_mesh->mTextureCoords[0u]), GL_STATIC_DRAW);
	glEnableVertexAttribArray(static_cast<int>(shader_bindings::texcoords));
	glVertexAttribPointer(static_cast<int>(shader_bindings::texcoords), 3, GL_FLOAT, false, 0, reinterpret_cast<GLvoid const*>(0x0));

	glBindBuffer(GL_ARRAY_BUFFER, 0u);

	sphere.indices_nb = static_cast<GLsizei>(assimp_sphere_mesh->mNumFaces * 3u);
	auto sphere_indices = std::make_unique<GLuint[]>(static_cast<size_t>(sphere.indices_nb));
	for (size_t i = 0u; i < assimp_sphere_mesh->mNumFaces; ++i) {
		auto const& face = assimp_sphere_mesh->mFaces[i];
		assert(face.mNumIndices == 3u);
		sphere_indices[3u * i + 0u] = face.mIndices[0u];
		sphere_indices[3u * i + 1u] = face.mIndices[1u];
		sphere_indices[3u * i + 2u] = face.mIndices[2u];
	}
	sphere.ibo = 0u;
	glGenBuffers(1, &sphere.ibo);
	assert(sphere.ibo != 0u);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphere.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<unsigned int>(sphere.indices_nb) * sizeof(GL_UNSIGNED_INT), reinterpret_cast<GLvoid const*>(sphere_indices.get()), GL_STATIC_DRAW);
	sphere_indices.reset(nullptr);

	glBindVertexArray(0u);
	glBindBuffer(GL_ARRAY_BUFFER, 0u);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0u);

	return sphere;
}
