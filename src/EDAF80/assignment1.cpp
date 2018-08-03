#include "assignment1.hpp"

#include "config.hpp"
#include "external/glad/glad.h"
#include "core/Bonobo.h"
#include "core/FPSCamera.h"
#include "core/helpers.hpp"
#include "core/InputHandler.h"
#include "core/Log.h"
#include "core/LogView.h"
#include "core/Misc.h"
#include "core/node.hpp"
#include "core/opengl.hpp"
#include "core/ShaderProgramManager.hpp"
#include "core/utils.h"
#include "core/various.hpp"
#include "core/Window.h"
#include <imgui.h>
#include "external/imgui_impl_glfw_gl3.h"

#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <array>
#include <chrono>
#include <cstdlib>
#include <unordered_map>
#include <stack>
#include <stdexcept>
#include <vector>


edaf80::Assignment1::Assignment1()
{
	Log::View::Init();

	window = Window::Create("EDAF80: Assignment 1", config::resolution_x,
	                        config::resolution_y, config::msaa_rate, false);
	if (window == nullptr) {
		Log::View::Destroy();
		throw std::runtime_error("Failed to get a window: aborting!");
	}
	inputHandler = new InputHandler();
	window->SetInputHandler(inputHandler);
}

edaf80::Assignment1::~Assignment1()
{
	delete inputHandler;
	inputHandler = nullptr;

	Window::Destroy(window);
	window = nullptr;

	Log::View::Destroy();
}

void
edaf80::Assignment1::run()
{
	// Load the sphere geometry
	auto const objects = bonobo::loadObjects("sphere.obj");
	if (objects.empty())
		return;
	auto const& sphere = objects.front();

	// Set up the camera
	FPSCameraf mCamera(0.5f * glm::half_pi<float>(),
	                   static_cast<float>(config::resolution_x) / static_cast<float>(config::resolution_y),
	                   0.01f, 1000.0f);
	mCamera.mWorld.SetTranslate(glm::vec3(0.0f, 0.0f, 6.0f));
	mCamera.mMouseSensitivity = 0.003f;
	mCamera.mMovementSpeed = 0.25f * 12.0f;
	window->SetCamera(&mCamera);

	// Create the shader program
	ShaderProgramManager program_manager;
	GLuint shader = 0u;
	program_manager.CreateAndRegisterProgram({ { ShaderType::vertex, "EDAF80/default.vert" },
	                                           { ShaderType::fragment, "EDAF80/default.frag" } },
	                                         shader);
	if (shader == 0u) {
		LogError("Failed to load shader");
		return;
	}

	// Load the sun's texture
	auto sun_texture = bonobo::loadTexture2D("sunmap.png");

	auto sun = Node();
	sun.set_geometry(sphere);
	sun.set_program(shader, [](GLuint /*program*/){});
	//
	// Todo: Attach a texture to the sun
	//

	auto world = Node();
	world.add_child(&sun);


	//
	// Todo: Create an Earth node
	//


	glEnable(GL_DEPTH_TEST);

	f64 ddeltatime;
	size_t fpsSamples = 0;
	double nowTime, lastTime = GetTimeSeconds();
	double fpsNextTick = lastTime + 1.0;

	bool show_logs = true;
	bool show_gui = true;

	while (!glfwWindowShouldClose(window->GetGLFW_Window())) {
		nowTime = GetTimeSeconds();
		ddeltatime = nowTime - lastTime;
		if (nowTime > fpsNextTick) {
			fpsNextTick += 1.0;
			fpsSamples = 0;
		}
		fpsSamples++;

		auto& io = ImGui::GetIO();
		inputHandler->SetUICapture(io.WantCaptureMouse, io.WantCaptureKeyboard);

		glfwPollEvents();
		inputHandler->Advance();
		mCamera.Update(ddeltatime, *inputHandler);

		if (inputHandler->GetKeycodeState(GLFW_KEY_F3) & JUST_RELEASED)
			show_logs = !show_logs;
		if (inputHandler->GetKeycodeState(GLFW_KEY_F2) & JUST_RELEASED)
			show_gui = !show_gui;

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
		matrix_stack.push(glm::mat4(1.0f));
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

		if (show_logs)
			Log::View::Render();
		if (show_gui)
			ImGui::Render();

		window->Swap();
		lastTime = nowTime;
	}
}

int main()
{
	Bonobo::Init();
	try {
		edaf80::Assignment1 assignment1;
		assignment1.run();
	}
	catch (std::runtime_error const& e) {
		LogError(e.what());
	}
	Bonobo::Destroy();
}
