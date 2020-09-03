#include "assignment1.hpp"
#include "config.hpp"
#include "parametric_shapes.hpp"
#include "core/Bonobo.h"
#include "core/FPSCamera.h"
#include "core/helpers.hpp"
#include "core/node.hpp"
#include "core/ShaderProgramManager.hpp"

#include <imgui.h>

#include <clocale>
#include <cstdlib>


int main()
{
	std::setlocale(LC_ALL, "");

	using namespace std::literals::chrono_literals;

	//
	// Set up the framework
	//
	Bonobo framework;

	//
	// Set up the camera
	//
	InputHandler input_handler;
	FPSCameraf camera(0.5f * glm::half_pi<float>(),
	                  static_cast<float>(config::resolution_x) / static_cast<float>(config::resolution_y),
	                  0.01f, 1000.0f);
	camera.mWorld.SetTranslate(glm::vec3(0.0f, 0.0f, 6.0f));
	camera.mWorld.LookAt(glm::vec3(0.0f));
	camera.mMouseSensitivity = 0.003f;
	camera.mMovementSpeed = 3.0f; // 3 m/s => 10.8 km/h

	//
	// Create the window
	//
	WindowManager& window_manager = framework.GetWindowManager();
	WindowManager::WindowDatum window_datum{ input_handler, camera, config::resolution_x, config::resolution_y, 0, 0, 0, 0};
	GLFWwindow* window = window_manager.CreateGLFWWindow("EDAF80: Assignment 1", window_datum, config::msaa_rate);
	if (window == nullptr) {
		LogError("Failed to get a window: exiting.");

		return EXIT_FAILURE;
	}

	//
	// Load the sphere geometry
	//
	std::vector<bonobo::mesh_data> const objects = bonobo::loadObjects(config::resources_path("scenes/sphere.obj"));
	if (objects.empty()) {
		LogError("Failed to load the sphere geometry: exiting.");

		return EXIT_FAILURE;
	}
	bonobo::mesh_data const& sphere = objects.front();
	auto const saturn_ring_shape = parametric_shapes::createCircleRing(0.675f, 0.45f, 80u, 8u);


	//
	// Create the shader program
	//
	ShaderProgramManager program_manager;
	GLuint celestial_body_shader = 0u;
	program_manager.CreateAndRegisterProgram("Celestial Body",
	                                         { { ShaderType::vertex, "EDAF80/default.vert" },
	                                           { ShaderType::fragment, "EDAF80/default.frag" } },
	                                         celestial_body_shader);
	if (celestial_body_shader == 0u) {
		LogError("Failed to generate the “Celestial Body” shader program: exiting.");

		Log::View::Destroy();
		Log::Destroy();

		return EXIT_FAILURE;
	}
	GLuint celestial_ring_shader = 0u;
	program_manager.CreateAndRegisterProgram("Celestial Ring",
	                                         { { ShaderType::vertex, "EDAF80/celestial_ring.vert" },
	                                           { ShaderType::fragment, "EDAF80/celestial_ring.frag" } },
	                                         celestial_ring_shader);
	if (celestial_ring_shader == 0u) {
		LogError("Failed to generate the “Celestial Ring” shader program: exiting.");

		Log::View::Destroy();
		Log::Destroy();

		return EXIT_FAILURE;
	}


	//
	// Define all the celestial bodies constants.
	//
	glm::vec3 const sun_scale{1.0f};
	SpinConfiguration const sun_spin{glm::radians(0.0f), glm::two_pi<float>() / 6.0f};

	glm::vec3 const mercury_scale{0.02f};
	SpinConfiguration const mercury_spin{glm::radians(2.0f), glm::two_pi<float>() / 180.0f};
	OrbitConfiguration const mercury_orbit{2.0f, glm::radians(3.4f), glm::two_pi<float>() / 4.0f};

	glm::vec3 const venus_scale{0.05f};
	SpinConfiguration const venus_spin{glm::radians(177.0f), glm::two_pi<float>() / 600.0f};
	OrbitConfiguration const venus_orbit{3.0f, glm::radians(3.9f), glm::two_pi<float>() / 12.0f};

	glm::vec3 const earth_scale{0.05f};
	SpinConfiguration const earth_spin{glm::radians(23.0f), glm::two_pi<float>() / 3.0f};
	OrbitConfiguration const earth_orbit{4.0f, glm::radians(7.2f), glm::two_pi<float>() / 20.0f};

	glm::vec3 const moon_scale{0.01f};
	SpinConfiguration const moon_spin{glm::radians(6.7f), glm::two_pi<float>() / 90.0f};
	OrbitConfiguration const moon_orbit{0.2f, glm::radians(-66.0f), glm::two_pi<float>() / 1.3f};

	glm::vec3 const mars_scale{0.03f};
	SpinConfiguration const mars_spin{glm::radians(25.0f), glm::two_pi<float>() / 3.0f};
	OrbitConfiguration const mars_orbit{5.0f, glm::radians(5.7f), glm::two_pi<float>() / 36.0f};

	glm::vec3 const jupiter_scale{0.5f};
	SpinConfiguration const jupiter_spin{glm::radians(3.1f), glm::two_pi<float>() / 1.0f};
	OrbitConfiguration const jupiter_orbit{13.0f, glm::radians(6.1f), glm::two_pi<float>() / 220.0f};

	glm::vec3 const saturn_scale{0.4f};
	SpinConfiguration const saturn_spin{glm::radians(27.0f), glm::two_pi<float>() / 1.2f};
	OrbitConfiguration const saturn_orbit{16.0f, glm::radians(5.5f), glm::two_pi<float>() / 400.0f};
	glm::vec2 const saturn_ring_scale{1.0f, 1.25f};

	glm::vec3 const uranus_scale{0.2f};
	SpinConfiguration const uranus_spin{glm::radians(98.0f), glm::two_pi<float>() / 2.0f};
	OrbitConfiguration const uranus_orbit{18.0f, glm::radians(6.5f), glm::two_pi<float>() / 1680.0f};

	glm::vec3 const neptune_scale{0.2f};
	SpinConfiguration const neptune_spin{glm::radians(28.0f), glm::two_pi<float>() / 2.0f};
	OrbitConfiguration const neptune_orbit{19.0f, glm::radians(6.4f), glm::two_pi<float>() / 3200.0f};


	//
	// Load all textures.
	//
	GLuint const sun_texture = bonobo::loadTexture2D(config::resources_path("planets/2k_sun.jpg"));
	GLuint const mercury_texture = bonobo::loadTexture2D(config::resources_path("planets/2k_mercury.jpg"));
	GLuint const venus_texture = bonobo::loadTexture2D(config::resources_path("planets/2k_venus_atmosphere.jpg"));
	GLuint const earth_texture = bonobo::loadTexture2D(config::resources_path("planets/2k_earth_daymap.jpg"));
	GLuint const moon_texture = bonobo::loadTexture2D(config::resources_path("planets/2k_moon.jpg"));
	GLuint const mars_texture = bonobo::loadTexture2D(config::resources_path("planets/2k_mars.jpg"));
	GLuint const jupiter_texture = bonobo::loadTexture2D(config::resources_path("planets/2k_jupiter.jpg"));
	GLuint const saturn_texture = bonobo::loadTexture2D(config::resources_path("planets/2k_saturn.jpg"));
	GLuint const saturn_ring_texture = bonobo::loadTexture2D(config::resources_path("planets/2k_saturn_ring_alpha.png"));
	GLuint const uranus_texture = bonobo::loadTexture2D(config::resources_path("planets/2k_uranus.jpg"));
	GLuint const neptune_texture = bonobo::loadTexture2D(config::resources_path("planets/2k_neptune.jpg"));


	//
	// Set up the sun node and other related attributes
	//
	Node sun;
	sun.set_geometry(sphere);
	sun.set_program(&celestial_body_shader);
	sun.add_texture("diffuse_texture", sun_texture, GL_TEXTURE_2D);


	glClearDepthf(1.0f);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glEnable(GL_DEPTH_TEST);


	auto last_time = std::chrono::high_resolution_clock::now();


	bool pause_animation = false;
	bool show_logs = true;
	bool show_gui = true;

	while (!glfwWindowShouldClose(window)) {
		//
		// Compute timings information
		//
		auto const now_time = std::chrono::high_resolution_clock::now();
		auto const delta_time_us = std::chrono::duration_cast<std::chrono::microseconds>(now_time - last_time);
		last_time = now_time;


		//
		// Process inputs
		//
		glfwPollEvents();

		ImGuiIO const& io = ImGui::GetIO();
		input_handler.SetUICapture(io.WantCaptureMouse, io.WantCaptureKeyboard);
		input_handler.Advance();
		camera.Update(delta_time_us, input_handler);

		if (input_handler.GetKeycodeState(GLFW_KEY_F3) & JUST_RELEASED)
			show_logs = !show_logs;
		if (input_handler.GetKeycodeState(GLFW_KEY_F2) & JUST_RELEASED)
			show_gui = !show_gui;
		if (input_handler.GetKeycodeState(GLFW_KEY_F11) & JUST_RELEASED)
			window_manager.ToggleFullscreenStatusForWindow(window);


		// Retrieve the actual framebuffer size: for HiDPI monitors,
		// you might end up with a framebuffer larger than what you
		// actually asked for. For example, if you ask for a 1920x1080
		// framebuffer, you might get a 3840x2160 one instead.
		// Also it might change as the user drags the window between
		// monitors with different DPIs, or if the fullscreen status is
		// being toggled.
		int framebuffer_width, framebuffer_height;
		glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
		glViewport(0, 0, framebuffer_width, framebuffer_height);


		//
		// Start a new frame for Dear ImGui
		//
		window_manager.NewImGuiFrame();


		//
		// Clear the screen
		//
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);


		//
		// Add controls to the scene.
		//
		bool const opened = ImGui::Begin("Scene controls", nullptr, ImGuiWindowFlags_None);
		if (opened)
		{
			ImGui::Checkbox("Pause the animation", &pause_animation);
		}
		ImGui::End();
		auto const animation_delta_time_us = !pause_animation ? delta_time_us : 0us;


		//
		// Traverse the scene graph and render all nodes
		//
		// TODO: Replace this explicit rendering of the Sun with a
		// traversal of the scene graph and rendering of all its nodes.
		sun.render(camera.GetWorldToClipMatrix());


		//
		// Display Dear ImGui windows
		//
		if (show_logs)
			Log::View::Render();
		if (show_gui)
			window_manager.RenderImGuiFrame();


		//
		// Queue the computed frame for display on screen
		//
		glfwSwapBuffers(window);
	}

	glDeleteTextures(1, &neptune_texture);
	glDeleteTextures(1, &uranus_texture);
	glDeleteTextures(1, &saturn_ring_texture);
	glDeleteTextures(1, &saturn_texture);
	glDeleteTextures(1, &jupiter_texture);
	glDeleteTextures(1, &mars_texture);
	glDeleteTextures(1, &moon_texture);
	glDeleteTextures(1, &earth_texture);
	glDeleteTextures(1, &venus_texture);
	glDeleteTextures(1, &mars_texture);
	glDeleteTextures(1, &sun_texture);

	return EXIT_SUCCESS;
}
