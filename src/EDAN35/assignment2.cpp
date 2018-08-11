#include "assignment2.hpp"

#include "config.hpp"
#include "core/Bonobo.h"
#include "core/FPSCamera.h"
#include "core/GLStateInspection.h"
#include "core/GLStateInspectionView.h"
#include "core/helpers.hpp"
#include "core/Log.h"
#include "core/LogView.h"
#include "core/Misc.h"
#include "core/node.hpp"
#include "core/ShaderProgramManager.hpp"

#include <imgui.h>
#include <external/imgui_impl_glfw_gl3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <tinyfiledialogs.h>

#include <array>
#include <cstdlib>
#include <stdexcept>

enum class polygon_mode_t : unsigned int {
	fill = 0u,
	line,
	point
};

static polygon_mode_t get_next_mode(polygon_mode_t mode)
{
	return static_cast<polygon_mode_t>((static_cast<unsigned int>(mode) + 1u) % 3u);
}

namespace constant
{
	constexpr uint32_t shadowmap_res_x = 1024;
	constexpr uint32_t shadowmap_res_y = 1024;

	constexpr size_t lights_nb           = 4;
	constexpr float  light_intensity     = 720000.0f;
	constexpr float  light_angle_falloff = glm::radians(37.0f);
	constexpr float  light_cutoff        = 0.05f;
}

static bonobo::mesh_data loadCone();

edan35::Assignment2::Assignment2() :
	mCamera(0.5f * glm::half_pi<float>(),
	        static_cast<float>(config::resolution_x) / static_cast<float>(config::resolution_y),
	        0.01f, 1000.0f),
	inputHandler(), mWindowManager(), window(nullptr)
{
	Log::View::Init();

	WindowManager::WindowDatum window_datum{ inputHandler, mCamera, config::resolution_x, config::resolution_y, 0, 0, 0, 0};

	window = mWindowManager.CreateWindow("EDAN35: Assignment 2", window_datum, config::msaa_rate);
	if (window == nullptr) {
		Log::View::Destroy();
		throw std::runtime_error("Failed to get a window: aborting!");
	}

	GLStateInspection::Init();
	GLStateInspection::View::Init();

	bonobo::init();
}

edan35::Assignment2::~Assignment2()
{
	bonobo::deinit();

	GLStateInspection::View::Destroy();
	GLStateInspection::Destroy();

	Log::View::Destroy();
}

void
edan35::Assignment2::run()
{
	// Load the geometry of Sponza
	auto const sponza_geometry = bonobo::loadObjects("../crysponza/sponza.obj");
	if (sponza_geometry.empty()) {
		LogError("Failed to load the Sponza model");
		return;
	}
	std::vector<Node> sponza_elements;
	sponza_elements.reserve(sponza_geometry.size());
	for (auto const& shape : sponza_geometry) {
		Node node;
		node.set_geometry(shape);
		sponza_elements.push_back(node);
	}

	auto const cone_geometry = loadCone();
	Node cone;
	cone.set_geometry(cone_geometry);

	//
	// Setup the camera
	//
	mCamera.mWorld.SetTranslate(glm::vec3(0.0f, 100.0f, 180.0f));
	mCamera.mMouseSensitivity = 0.003f;
	mCamera.mMovementSpeed = 0.25f;

	//
	// Load all the shader programs used
	//
	ShaderProgramManager program_manager;
	GLuint fallback_shader = 0u;
	program_manager.CreateAndRegisterProgram({ { ShaderType::vertex, "EDAF80/fallback.vert" },
	                                           { ShaderType::fragment, "EDAF80/fallback.frag" } },
	                                         fallback_shader);
	if (fallback_shader == 0u) {
		LogError("Failed to load fallback shader");
		return;
	}

	GLuint fill_gbuffer_shader = 0u;
	program_manager.CreateAndRegisterProgram({ { ShaderType::vertex, "EDAN35/fill_gbuffer.vert" },
	                                           { ShaderType::fragment, "EDAN35/fill_gbuffer.frag" } },
	                                         fill_gbuffer_shader);
	if (fill_gbuffer_shader == 0u) {
		LogError("Failed to load G-buffer filling shader");
		return;
	}

	GLuint fill_shadowmap_shader = 0u;
	program_manager.CreateAndRegisterProgram({ { ShaderType::vertex, "EDAN35/fill_shadowmap.vert" },
	                                           { ShaderType::fragment, "EDAN35/fill_shadowmap.frag" } },
	                                         fill_shadowmap_shader);
	if (fill_shadowmap_shader == 0u) {
		LogError("Failed to load shadowmap filling shader");
		return;
	}

	GLuint accumulate_lights_shader = 0u;
	program_manager.CreateAndRegisterProgram({ { ShaderType::vertex, "EDAN35/accumulate_lights.vert" },
	                                           { ShaderType::fragment, "EDAN35/accumulate_lights.frag" } },
	                                         accumulate_lights_shader);
	if (accumulate_lights_shader == 0u) {
		LogError("Failed to load lights accumulating shader");
		return;
	}

	GLuint resolve_deferred_shader = 0u;
	program_manager.CreateAndRegisterProgram({ { ShaderType::vertex, "EDAN35/resolve_deferred.vert" },
	                                           { ShaderType::fragment, "EDAN35/resolve_deferred.frag" } },
	                                         resolve_deferred_shader);
	if (resolve_deferred_shader == 0u) {
		LogError("Failed to load deferred resolution shader");
		return;
	}

	auto const set_uniforms = [](GLuint /*program*/){};

	int framebuffer_width, framebuffer_height;
	glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);

	//
	// Setup textures
	//
	auto const diffuse_texture                     = bonobo::createTexture(framebuffer_width, framebuffer_height);
	auto const specular_texture                    = bonobo::createTexture(framebuffer_width, framebuffer_height);
	auto const normal_texture                      = bonobo::createTexture(framebuffer_width, framebuffer_height);
	auto const light_diffuse_contribution_texture  = bonobo::createTexture(framebuffer_width, framebuffer_height);
	auto const light_specular_contribution_texture = bonobo::createTexture(framebuffer_width, framebuffer_height);
	auto const depth_texture                       = bonobo::createTexture(framebuffer_width, framebuffer_height, GL_TEXTURE_2D, GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT);
	auto const shadowmap_texture                   = bonobo::createTexture(constant::shadowmap_res_x, constant::shadowmap_res_y, GL_TEXTURE_2D, GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT);


	//
	// Setup FBOs
	//
	auto const deferred_fbo  = bonobo::createFBO({diffuse_texture, specular_texture, normal_texture}, depth_texture);
	auto const shadowmap_fbo = bonobo::createFBO({}, shadowmap_texture);
	auto const light_fbo     = bonobo::createFBO({light_diffuse_contribution_texture, light_specular_contribution_texture}, depth_texture);

	//
	// Setup samplers
	//
	auto const default_sampler = bonobo::createSampler([](GLuint sampler){
		glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	});
	auto const depth_sampler = bonobo::createSampler([](GLuint sampler){
		glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	});
	auto const shadow_sampler = bonobo::createSampler([](GLuint sampler){
		glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glSamplerParameteri(sampler, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		glSamplerParameteri(sampler, GL_TEXTURE_COMPARE_FUNC, GL_LESS);
		GLfloat border_color[4] = { 1.0f, 0.0f, 0.0f, 0.0f};
		glSamplerParameterfv(sampler, GL_TEXTURE_BORDER_COLOR, border_color);
	});
	auto const bind_texture_with_sampler = [](GLenum target, unsigned int slot, GLuint program, std::string const& name, GLuint texture, GLuint sampler){
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(target, texture);
		glUniform1i(glGetUniformLocation(program, name.c_str()), static_cast<GLint>(slot));
		glBindSampler(slot, sampler);
	};


	//
	// Setup lights properties
	//
	std::array<TRSTransform<float, glm::defaultp>, constant::lights_nb> lightTransforms;
	std::array<glm::vec3, constant::lights_nb> lightColors;
	int lights_nb = static_cast<int>(constant::lights_nb);
	bool are_lights_paused = false;

	for (size_t i = 0; i < static_cast<size_t>(lights_nb); ++i) {
		lightTransforms[i].SetTranslate(glm::vec3(0.0f, 125.0f, 0.0f));
		lightColors[i] = glm::vec3(0.5f + 0.5f * (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)),
		                           0.5f + 0.5f * (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)),
		                           0.5f + 0.5f * (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)));
	}

	TRSTransform<f32, glm::defaultp> coneScaleTransform = TRSTransform<f32, glm::defaultp>();
	coneScaleTransform.SetScale(glm::vec3(std::sqrt(constant::light_intensity / constant::light_cutoff)));

	TRSTransform<f32, glm::defaultp> lightOffsetTransform = TRSTransform<f32, glm::defaultp>();
	lightOffsetTransform.SetTranslate(glm::vec3(0.0f, 0.0f, -40.0f));

	auto lightProjection = glm::perspective(0.5f * glm::pi<float>(),
	                                        static_cast<float>(constant::shadowmap_res_x) / static_cast<float>(constant::shadowmap_res_y),
	                                        1.0f, 10000.0f);


	auto seconds_nb = 0.0f;


	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);


	double ddeltatime;
	size_t fpsSamples = 0;
	double nowTime, lastTime = GetTimeMilliseconds();
	double fpsNextTick = lastTime + 1000.0;
	bool show_textures = true;

	bool show_logs = true;
	bool show_gui = true;
	bool shader_reload_failed = false;

	while (!glfwWindowShouldClose(window)) {
		nowTime = GetTimeMilliseconds();
		ddeltatime = nowTime - lastTime;
		if (nowTime > fpsNextTick) {
			fpsNextTick += 1000.0;
			fpsSamples = 0;
		}
		fpsSamples++;
		if (!are_lights_paused)
			seconds_nb += static_cast<float>(ddeltatime / 1000.0);

		auto& io = ImGui::GetIO();
		inputHandler.SetUICapture(io.WantCaptureMouse, io.WantCaptureKeyboard);

		glfwPollEvents();
		inputHandler.Advance();
		mCamera.Update(ddeltatime, inputHandler);

		if (inputHandler.GetKeycodeState(GLFW_KEY_F3) & JUST_RELEASED)
			show_logs = !show_logs;
		if (inputHandler.GetKeycodeState(GLFW_KEY_F2) & JUST_RELEASED)
			show_gui = !show_gui;
		if (inputHandler.GetKeycodeState(GLFW_KEY_R) & JUST_PRESSED) {
			shader_reload_failed = !program_manager.ReloadAllPrograms();
			if (shader_reload_failed)
				tinyfd_notifyPopup("Shader Program Reload Error",
				                   "An error occurred while reloading shader programs; see the logs for details.\n"
				                   "Rendering is suspended until the issue is solved. Once fixed, just reload the shaders again.",
				                   "error");
		}

		ImGui_ImplGlfwGL3_NewFrame();


		if (!shader_reload_failed) {
			glDepthFunc(GL_LESS);
			//
			// Pass 1: Render scene into the g-buffer
			//
			glBindFramebuffer(GL_FRAMEBUFFER, deferred_fbo);
			GLenum const deferred_draw_buffers[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
			glDrawBuffers(3, deferred_draw_buffers);
			auto const status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			if (status != GL_FRAMEBUFFER_COMPLETE)
				LogError("Something went wrong with framebuffer %u", deferred_fbo);
			int framebuffer_width, framebuffer_height;
			glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
			glViewport(0, 0, framebuffer_width, framebuffer_height);
			glClear(GL_DEPTH_BUFFER_BIT);
			// XXX: Is any other clearing needed?

			GLStateInspection::CaptureSnapshot("Filling Pass");

			for (auto const& element : sponza_elements)
				element.render(mCamera.GetWorldToClipMatrix(), element.get_transform(), fill_gbuffer_shader, set_uniforms);



			glCullFace(GL_FRONT);
			//
			// Pass 2: Generate shadowmaps and accumulate lights' contribution
			//
			glBindFramebuffer(GL_FRAMEBUFFER, light_fbo);
			GLenum light_draw_buffers[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
			glDrawBuffers(2, light_draw_buffers);
			glViewport(0, 0, framebuffer_width, framebuffer_height);
			// XXX: Is any clearing needed?
			for (size_t i = 0; i < static_cast<size_t>(lights_nb); ++i) {
				auto& lightTransform = lightTransforms[i];
				lightTransform.SetRotate(seconds_nb * 0.1f + i * 1.57f, glm::vec3(0.0f, 1.0f, 0.0f));

				auto light_matrix = lightProjection * lightOffsetTransform.GetMatrixInverse() * lightTransform.GetMatrixInverse();

				//
				// Pass 2.1: Generate shadow map for light i
				//
				glBindFramebuffer(GL_FRAMEBUFFER, shadowmap_fbo);
				glViewport(0, 0, constant::shadowmap_res_x, constant::shadowmap_res_y);
				// XXX: Is any clearing needed?

				GLStateInspection::CaptureSnapshot("Shadow Map Generation");

				for (auto const& element : sponza_elements)
					element.render(light_matrix, glm::mat4(1.0f), fill_gbuffer_shader, set_uniforms);


				glEnable(GL_BLEND);
				glDepthFunc(GL_GREATER);
				glDepthMask(GL_FALSE);
				glBlendEquationSeparate(GL_FUNC_ADD, GL_MIN);
				glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ONE);
				//
				// Pass 2.2: Accumulate light i contribution
				glBindFramebuffer(GL_FRAMEBUFFER, light_fbo);
				glDrawBuffers(2, light_draw_buffers);
				glUseProgram(accumulate_lights_shader);
				glViewport(0, 0, framebuffer_width, framebuffer_height);
				// XXX: Is any clearing needed?

				auto const spotlight_set_uniforms = [framebuffer_width,framebuffer_height,this,&light_matrix,&lightColors,&lightTransform,&i](GLuint program){
					glUniform2f(glGetUniformLocation(program, "inv_res"),
					            1.0f / static_cast<float>(framebuffer_width),
					            1.0f / static_cast<float>(framebuffer_height));
					glUniformMatrix4fv(glGetUniformLocation(program, "view_projection_inverse"), 1, GL_FALSE,
					                   glm::value_ptr(mCamera.GetClipToWorldMatrix()));
					glUniform3fv(glGetUniformLocation(program, "camera_position"), 1,
					                   glm::value_ptr(mCamera.mWorld.GetTranslation()));
					glUniformMatrix4fv(glGetUniformLocation(program, "shadow_view_projection"), 1, GL_FALSE,
					                   glm::value_ptr(light_matrix));
					glUniform3fv(glGetUniformLocation(program, "light_color"), 1, glm::value_ptr(lightColors[i]));
					glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(lightTransform.GetTranslation()));
					glUniform3fv(glGetUniformLocation(program, "light_direction"), 1, glm::value_ptr(lightTransform.GetFront()));
					glUniform1f(glGetUniformLocation(program, "light_intensity"), constant::light_intensity);
					glUniform1f(glGetUniformLocation(program, "light_angle_falloff"), constant::light_angle_falloff);
					glUniform2f(glGetUniformLocation(program, "shadowmap_texel_size"),
					            1.0f / static_cast<float>(constant::shadowmap_res_x),
					            1.0f / static_cast<float>(constant::shadowmap_res_y));
				};

				bind_texture_with_sampler(GL_TEXTURE_2D, 0, accumulate_lights_shader, "depth_texture", depth_texture, depth_sampler);
				bind_texture_with_sampler(GL_TEXTURE_2D, 1, accumulate_lights_shader, "normal_texture", normal_texture, default_sampler);
				bind_texture_with_sampler(GL_TEXTURE_2D, 2, accumulate_lights_shader, "shadow_texture", shadowmap_texture, shadow_sampler);

				GLStateInspection::CaptureSnapshot("Accumulating");

				cone.render(mCamera.GetWorldToClipMatrix(),
				            lightTransform.GetMatrix() * lightOffsetTransform.GetMatrix() * coneScaleTransform.GetMatrix(),
				            accumulate_lights_shader, spotlight_set_uniforms);

				glBindSampler(2u, 0u);
				glBindSampler(1u, 0u);
				glBindSampler(0u, 0u);

				glDepthMask(GL_TRUE);
				glDepthFunc(GL_LESS);
				glDisable(GL_BLEND);
			}


			glCullFace(GL_BACK);
			glDepthFunc(GL_ALWAYS);
			//
			// Pass 3: Compute final image using both the g-buffer and  the light accumulation buffer
			//
			glBindFramebuffer(GL_FRAMEBUFFER, 0u);
			glUseProgram(resolve_deferred_shader);
			glViewport(0, 0, framebuffer_width, framebuffer_height);
			// XXX: Is any clearing needed?

			bind_texture_with_sampler(GL_TEXTURE_2D, 0, resolve_deferred_shader, "diffuse_texture", diffuse_texture, default_sampler);
			bind_texture_with_sampler(GL_TEXTURE_2D, 1, resolve_deferred_shader, "specular_texture", specular_texture, default_sampler);
			bind_texture_with_sampler(GL_TEXTURE_2D, 2, resolve_deferred_shader, "light_d_texture", light_diffuse_contribution_texture, default_sampler);
			bind_texture_with_sampler(GL_TEXTURE_2D, 3, resolve_deferred_shader, "light_s_texture", light_specular_contribution_texture, default_sampler);

			GLStateInspection::CaptureSnapshot("Resolve Pass");

			bonobo::drawFullscreen();

			glBindSampler(3, 0u);
			glBindSampler(2, 0u);
			glBindSampler(1, 0u);
			glBindSampler(0, 0u);
			glUseProgram(0u);
		}


		//
		// Pass 4: Draw wireframe cones on top of the final image for debugging purposes
		//
//		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//		for (size_t i = 0; i < lights_nb; ++i) {
//			cone.render(mCamera.GetWorldToClipMatrix(),
//			            lightTransforms[i].GetMatrix() * lightOffsetTransform.GetMatrix() * coneScaleTransform.GetMatrix(),
//			            fill_shadowmap_shader, set_uniforms);
//		}
//		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


		//
		// Output content of the g-buffer as well as of the shadowmap, for debugging purposes
		//
		if (show_textures) {
			bonobo::displayTexture({-0.95f, -0.95f}, {-0.55f, -0.55f}, diffuse_texture,                     default_sampler, {0, 1, 2, -1}, glm::uvec2(framebuffer_width, framebuffer_height));
			bonobo::displayTexture({-0.45f, -0.95f}, {-0.05f, -0.55f}, specular_texture,                    default_sampler, {0, 1, 2, -1}, glm::uvec2(framebuffer_width, framebuffer_height));
			bonobo::displayTexture({ 0.05f, -0.95f}, { 0.45f, -0.55f}, normal_texture,                      default_sampler, {0, 1, 2, -1}, glm::uvec2(framebuffer_width, framebuffer_height));
			bonobo::displayTexture({ 0.55f, -0.95f}, { 0.95f, -0.55f}, depth_texture,                       default_sampler, {0, 0, 0, -1}, glm::uvec2(framebuffer_width, framebuffer_height), &mCamera);
			bonobo::displayTexture({-0.95f,  0.55f}, {-0.55f,  0.95f}, shadowmap_texture,                   default_sampler, {0, 0, 0, -1}, glm::uvec2(framebuffer_width, framebuffer_height), &mCamera);
			bonobo::displayTexture({-0.45f,  0.55f}, {-0.05f,  0.95f}, light_diffuse_contribution_texture,  default_sampler, {0, 1, 2, -1}, glm::uvec2(framebuffer_width, framebuffer_height));
			bonobo::displayTexture({ 0.05f,  0.55f}, { 0.45f,  0.95f}, light_specular_contribution_texture, default_sampler, {0, 1, 2, -1}, glm::uvec2(framebuffer_width, framebuffer_height));
		}
		//
		// Reset viewport back to normal
		//
		glViewport(0, 0, framebuffer_width, framebuffer_height);

		GLStateInspection::View::Render();

		bool opened = ImGui::Begin("Render Time", nullptr, ImVec2(120, 50), -1.0f, 0);
		if (opened)
			ImGui::Text("%.3f ms", ddeltatime);
		ImGui::End();

		opened = ImGui::Begin("Scene Controls", nullptr, ImVec2(350, 100), -1.0f, 0);
		if (opened) {
			ImGui::Checkbox("Pause lights", &are_lights_paused);
			ImGui::SliderInt("Number of lights", &lights_nb, 1, static_cast<int>(constant::lights_nb));
			ImGui::Checkbox("Show textures", &show_textures);
		}
		ImGui::End();

		if (show_logs)
			Log::View::Render();
		if (show_gui)
			ImGui::Render();

		glfwSwapBuffers(window);
		lastTime = nowTime;
	}

	glDeleteProgram(resolve_deferred_shader);
	resolve_deferred_shader = 0u;
	glDeleteProgram(accumulate_lights_shader);
	accumulate_lights_shader = 0u;
	glDeleteProgram(fill_shadowmap_shader);
	fill_shadowmap_shader = 0u;
	glDeleteProgram(fill_gbuffer_shader);
	fill_gbuffer_shader = 0u;
	glDeleteProgram(fallback_shader);
	fallback_shader = 0u;
}

int main()
{
	Bonobo::Init();
	try {
		edan35::Assignment2 assignment2;
		assignment2.run();
	} catch (std::runtime_error const& e) {
		LogError(e.what());
	}
	Bonobo::Destroy();
}

static
bonobo::mesh_data
loadCone()
{
	bonobo::mesh_data cone;
	cone.vertices_nb = 65;
	cone.drawing_mode = GL_TRIANGLE_STRIP;
	float vertexArrayData[65 * 3] = {
		0.f, 1.f, -1.f,
		0.f, 0.f, 0.f,
		0.38268f, 0.92388f, -1.f,
		0.f, 0.f, 0.f,
		0.70711f, 0.70711f, -1.f,
		0.f, 0.f, 0.f,
		0.92388f, 0.38268f, -1.f,
		0.f, 0.f, 0.f,
		1.f, 0.f, -1.f,
		0.f, 0.f, 0.f,
		0.92388f, -0.38268f, -1.f,
		0.f, 0.f, 0.f,
		0.70711f, -0.70711f, -1.f,
		0.f, 0.f, 0.f,
		0.38268f, -0.92388f, -1.f,
		0.f, 0.f, 0.f,
		0.f, -1.f, -1.f,
		0.f, 0.f, 0.f,
		-0.38268f, -0.92388f, -1.f,
		0.f, 0.f, 0.f,
		-0.70711f, -0.70711f, -1.f,
		0.f, 0.f, 0.f,
		-0.92388f, -0.38268f, -1.f,
		0.f, 0.f, 0.f,
		-1.f, 0.f, -1.f,
		0.f, 0.f, 0.f,
		-0.92388f, 0.38268f, -1.f,
		0.f, 0.f, 0.f,
		-0.70711f, 0.70711f, -1.f,
		0.f, 0.f, 0.f,
		-0.38268f, 0.92388f, -1.f,
		0.f, 1.f, -1.f,
		0.f, 1.f, -1.f,
		0.38268f, 0.92388f, -1.f,
		0.f, 1.f, -1.f,
		0.70711f, 0.70711f, -1.f,
		0.f, 0.f, -1.f,
		0.92388f, 0.38268f, -1.f,
		0.f, 0.f, -1.f,
		1.f, 0.f, -1.f,
		0.f, 0.f, -1.f,
		0.92388f, -0.38268f, -1.f,
		0.f, 0.f, -1.f,
		0.70711f, -0.70711f, -1.f,
		0.f, 0.f, -1.f,
		0.38268f, -0.92388f, -1.f,
		0.f, 0.f, -1.f,
		0.f, -1.f, -1.f,
		0.f, 0.f, -1.f,
		-0.38268f, -0.92388f, -1.f,
		0.f, 0.f, -1.f,
		-0.70711f, -0.70711f, -1.f,
		0.f, 0.f, -1.f,
		-0.92388f, -0.38268f, -1.f,
		0.f, 0.f, -1.f,
		-1.f, 0.f, -1.f,
		0.f, 0.f, -1.f,
		-0.92388f, 0.38268f, -1.f,
		0.f, 0.f, -1.f,
		-0.70711f, 0.70711f, -1.f,
		0.f, 0.f, -1.f,
		-0.38268f, 0.92388f, -1.f,
		0.f, 0.f, -1.f,
		0.f, 1.f, -1.f,
		0.f, 0.f, -1.f
	};

	glGenVertexArrays(1, &cone.vao);
	assert(cone.vao != 0u);
	glBindVertexArray(cone.vao);
	{
		glGenBuffers(1, &cone.bo);
		assert(cone.bo != 0u);
		glBindBuffer(GL_ARRAY_BUFFER, cone.bo);
		glBufferData(GL_ARRAY_BUFFER, cone.vertices_nb * 3 * sizeof(float), vertexArrayData, GL_STATIC_DRAW);

		glVertexAttribPointer(static_cast<int>(bonobo::shader_bindings::vertices), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(0x0));
		glEnableVertexAttribArray(static_cast<int>(bonobo::shader_bindings::vertices));

		glBindBuffer(GL_ARRAY_BUFFER, 0u);
	}
	glBindVertexArray(0u);

	return cone;
}
