#include "assignment4.hpp"

#include "config.hpp"
#include "external/glad/glad.h"
#include "core/Bonobo.h"
#include "core/FPSCamera.h"
#include "core/helpers.hpp"
#include "core/InputHandler.h"
#include "core/Log.h"
#include "core/LogView.h"
#include "core/Misc.h"
#include "core/utils.h"
#include "core/Window.h"
#include <imgui.h>
#include "external/imgui_impl_glfw_gl3.h"

#include "external/glad/glad.h"
#include <GLFW/glfw3.h>

#include <stdexcept>

edaf80::Assignment4::Assignment4()
{
	Log::View::Init();

	window = Window::Create("EDAF80: Assignment 4", config::resolution_x,
	                        config::resolution_y, config::msaa_rate, false);
	if (window == nullptr) {
		Log::View::Destroy();
		throw std::runtime_error("Failed to get a window: aborting!");
	}
	inputHandler = new InputHandler();
	window->SetInputHandler(inputHandler);
}

edaf80::Assignment4::~Assignment4()
{
	delete inputHandler;
	inputHandler = nullptr;

	Window::Destroy(window);
	window = nullptr;

	Log::View::Destroy();
}

void
edaf80::Assignment4::run()
{
	// Set up the camera
	FPSCameraf mCamera(bonobo::pi / 4.0f,
	                   static_cast<float>(config::resolution_x) / static_cast<float>(config::resolution_y),
	                   0.01f, 1000.0f);
	mCamera.mWorld.SetTranslate(glm::vec3(0.0f, 0.0f, 6.0f));
	mCamera.mMouseSensitivity = 0.003f;
	mCamera.mMovementSpeed = 0.025;
	window->SetCamera(&mCamera);

	// Create the shader programs
	auto fallback_shader = bonobo::createProgram("fallback.vert", "fallback.frag");
	if (fallback_shader == 0u) {
		LogError("Failed to load fallback shader");
		return;
	}
	auto const reload_shaders = [](){
		//
		// Todo: Insert the creation of other shader programs.
		//       (Check how it was done in assignment 3.)
		//
	};
	reload_shaders();

	//
	// Todo: Load your geometry
	//

	glEnable(GL_DEPTH_TEST);

	// Enable face culling to improve performance:
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_FRONT);
	//glCullFace(GL_BACK);


	f64 ddeltatime;
	size_t fpsSamples = 0;
	double nowTime, lastTime = GetTimeMilliseconds();
	double fpsNextTick = lastTime + 1000.0;

	while (!glfwWindowShouldClose(window->GetGLFW_Window())) {
		nowTime = GetTimeMilliseconds();
		ddeltatime = nowTime - lastTime;
		if (nowTime > fpsNextTick) {
			fpsNextTick += 1000.0;
			fpsSamples = 0;
		}
		fpsSamples++;

		auto& io = ImGui::GetIO();
		inputHandler->SetUICapture(io.WantCaptureMouse, io.WantCaptureMouse);

		glfwPollEvents();
		inputHandler->Advance();
		mCamera.Update(ddeltatime, *inputHandler);

		ImGui_ImplGlfwGL3_NewFrame();

		//
		// Todo: If you need to handle inputs, you can do it here
		//
		if (inputHandler->GetKeycodeState(GLFW_KEY_R) & JUST_PRESSED) {
			reload_shaders();
		}


		auto const window_size = window->GetDimensions();
		glViewport(0, 0, window_size.x, window_size.y);
		glClearDepthf(1.0f);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		//
		// Todo: Render all your geometry here.
		//

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		Log::View::Render();

		//
		// Todo: If you want a custom ImGUI window, you can set it up
		//       here
		//

		ImGui::Render();

		window->Swap();
		lastTime = nowTime;
	}

	//
	// Todo: Do not forget to delete your shader programs, by calling
	//       `glDeleteProgram($your_shader_program)` for each of them.
	//
}

int main()
{
	Bonobo::Init();
	try {
		edaf80::Assignment4 assignment4;
		assignment4.run();
	} catch (std::runtime_error const& e) {
		LogError(e.what());
	}
	Bonobo::Destroy();
}
