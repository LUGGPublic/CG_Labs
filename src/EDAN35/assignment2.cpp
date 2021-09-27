// Do not use intrinsic functions, which allows using `constexpr` on GLM
// functions.
#define GLM_FORCE_PURE 1

#include "assignment2.hpp"

#include "config.hpp"
#include "core/Bonobo.h"
#include "core/FPSCamera.h"
#include "core/helpers.hpp"
#include "core/node.hpp"
#include "core/opengl.hpp"
#include "core/ShaderProgramManager.hpp"

#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <tinyfiledialogs.h>

#include <array>
#include <clocale>
#include <cstdlib>
#include <stdexcept>

namespace constant
{
	constexpr uint32_t shadowmap_res_x = 1024;
	constexpr uint32_t shadowmap_res_y = 1024;

	constexpr float  scale_lengths       = 100.0f; // The scene is expressed in centimetres rather than metres, hence the x100.

	constexpr size_t lights_nb           = 4;
	constexpr float  light_intensity     = 72.0f * (scale_lengths * scale_lengths);
	constexpr float  light_angle_falloff = glm::radians(37.0f);
}

namespace
{
	template <class E> constexpr auto toU(E const& e)
	{
		return static_cast<std::underlying_type_t<E>>(e);
	}

	enum class Texture : uint32_t {
		DepthBuffer = 0u,
		ShadowMap,
		GBufferDiffuse,
		GBufferSpecular,
		GBufferWorldSpaceNormal,
		LightDiffuseContribution,
		LightSpecularContribution,
		Result,
		Count
	};
	using Textures = std::array<GLuint, toU(Texture::Count)>;
	Textures createTextures(GLsizei framebuffer_width, GLsizei framebuffer_height);

	enum class Sampler : uint32_t {
		Nearest = 0u,
		Linear,
		Mipmaps,
		Shadow,
		Count
	};
	using Samplers = std::array<GLuint, toU(Sampler::Count)>;
	Samplers createSamplers();

	enum class FBO : uint32_t {
		GBuffer = 0u,
		ShadowMap,
		LightAccumulation,
		Resolve,
		FinalWithDepth,
		Count
	};
	using FBOs = std::array<GLuint, toU(FBO::Count)>;
	FBOs createFramebufferObjects(Textures const& textures);

	enum class ElapsedTimeQuery : uint32_t {
		GbufferGeneration = 0u,
		ShadowMap0Generation,
		Light0Accumulation = ShadowMap0Generation + static_cast<uint32_t>(constant::lights_nb),
		Resolve = Light0Accumulation + static_cast<uint32_t>(constant::lights_nb),
		ConeWireframe,
		GUI,
		CopyToFramebuffer,
		Count
	};
	using ElapsedTimeQueries = std::array<GLuint, toU(ElapsedTimeQuery::Count)>;
	ElapsedTimeQueries createElapsedTimeQueries();

	bonobo::mesh_data loadCone();
} // namespace

edan35::Assignment2::Assignment2(WindowManager& windowManager) :
	mCamera(0.5f * glm::half_pi<float>(),
	        static_cast<float>(config::resolution_x) / static_cast<float>(config::resolution_y),
	        0.01f * constant::scale_lengths, 30.0f * constant::scale_lengths),
	inputHandler(), mWindowManager(windowManager), window(nullptr)
{
	WindowManager::WindowDatum window_datum{ inputHandler, mCamera, config::resolution_x, config::resolution_y, 0, 0, 0, 0};

	window = mWindowManager.CreateGLFWWindow("EDAN35: Assignment 2", window_datum, config::msaa_rate);
	if (window == nullptr) {
		throw std::runtime_error("Failed to get a window: aborting!");
	}

	bonobo::init();
}

edan35::Assignment2::~Assignment2()
{
	bonobo::deinit();
}

void
edan35::Assignment2::run()
{
	// Load the geometry of Sponza
	auto const sponza_geometry = bonobo::loadObjects(config::resources_path("sponza/sponza.obj"));
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
	mCamera.mWorld.SetTranslate(glm::vec3(0.0f, 1.0f, 1.8f) * constant::scale_lengths);
	mCamera.mMouseSensitivity = 0.003f;
	mCamera.mMovementSpeed = 3.0f * constant::scale_lengths; // 3 m/s => 10.8 km/h.

	//
	// Load all the shader programs used
	//
	ShaderProgramManager program_manager;
	GLuint fallback_shader = 0u;
	program_manager.CreateAndRegisterProgram("Fallback",
	                                         { { ShaderType::vertex, "common/fallback.vert" },
	                                           { ShaderType::fragment, "common/fallback.frag" } },
	                                         fallback_shader);
	if (fallback_shader == 0u) {
		LogError("Failed to load fallback shader");
		return;
	}

	GLuint fill_gbuffer_shader = 0u;
	program_manager.CreateAndRegisterProgram("Fill G-Buffer",
	                                         { { ShaderType::vertex, "EDAN35/fill_gbuffer.vert" },
	                                           { ShaderType::fragment, "EDAN35/fill_gbuffer.frag" } },
	                                         fill_gbuffer_shader);
	if (fill_gbuffer_shader == 0u) {
		LogError("Failed to load G-buffer filling shader");
		return;
	}

	GLuint fill_shadowmap_shader = 0u;
	program_manager.CreateAndRegisterProgram("Fill shadow map",
	                                         { { ShaderType::vertex, "EDAN35/fill_shadowmap.vert" },
	                                           { ShaderType::fragment, "EDAN35/fill_shadowmap.frag" } },
	                                         fill_shadowmap_shader);
	if (fill_shadowmap_shader == 0u) {
		LogError("Failed to load shadowmap filling shader");
		return;
	}

	GLuint accumulate_lights_shader = 0u;
	program_manager.CreateAndRegisterProgram("Accumulate light",
	                                         { { ShaderType::vertex, "EDAN35/accumulate_lights.vert" },
	                                           { ShaderType::fragment, "EDAN35/accumulate_lights.frag" } },
	                                         accumulate_lights_shader);
	if (accumulate_lights_shader == 0u) {
		LogError("Failed to load lights accumulating shader");
		return;
	}

	GLuint resolve_deferred_shader = 0u;
	program_manager.CreateAndRegisterProgram("Resolve deferred",
	                                         { { ShaderType::vertex, "EDAN35/resolve_deferred.vert" },
	                                           { ShaderType::fragment, "EDAN35/resolve_deferred.frag" } },
	                                         resolve_deferred_shader);
	if (resolve_deferred_shader == 0u) {
		LogError("Failed to load deferred resolution shader");
		return;
	}

	GLuint render_light_cones_shader = 0u;
	program_manager.CreateAndRegisterProgram("Render light cones",
	                                         { { ShaderType::vertex, "EDAN35/render_light_cones.vert" },
	                                           { ShaderType::fragment, "EDAN35/render_light_cones.frag" } },
	                                         render_light_cones_shader);
	if (render_light_cones_shader == 0u) {
		LogError("Failed to load light cones rendering shader");
		return;
	}

	auto const set_uniforms = [](GLuint /*program*/){};

	int framebuffer_width, framebuffer_height;
	glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);

	//
	// Setup OpenGL objects
	// Look further down in this file to see the implementation of those functions.
	//
	Textures const textures = createTextures(framebuffer_width, framebuffer_height);
	FBOs const fbos = createFramebufferObjects(textures);
	Samplers const samplers = createSamplers();
	ElapsedTimeQueries const elapsed_time_queries = createElapsedTimeQueries();

	auto const bind_texture_with_sampler = [](GLenum target, unsigned int slot, GLuint program, std::string const& name, GLuint texture, GLuint sampler){
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(target, texture);
		glUniform1i(glGetUniformLocation(program, name.c_str()), static_cast<GLint>(slot));
		glBindSampler(slot, sampler);
	};


	//
	// Setup lights properties
	//
	std::array<TRSTransformf, constant::lights_nb> lightTransforms;
	std::array<glm::vec3, constant::lights_nb> lightColors;
	int lights_nb = static_cast<int>(constant::lights_nb);
	bool are_lights_paused = false;

	for (size_t i = 0; i < static_cast<size_t>(lights_nb); ++i) {
		lightTransforms[i].SetTranslate(glm::vec3(0.0f, 1.25f, 0.0f) * constant::scale_lengths);
		lightColors[i] = glm::vec3(0.5f + 0.5f * (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)),
		                           0.5f + 0.5f * (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)),
		                           0.5f + 0.5f * (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)));
	}

	float const lightProjectionNearPlane = 0.01f * constant::scale_lengths;
	float const lightProjectionFarPlane = 20.0f * constant::scale_lengths;
	auto lightProjection = glm::perspective(0.5f * glm::pi<float>(),
	                                        static_cast<float>(constant::shadowmap_res_x) / static_cast<float>(constant::shadowmap_res_y),
	                                        lightProjectionNearPlane, lightProjectionFarPlane);

	TRSTransformf coneScaleTransform;
	coneScaleTransform.SetScale(glm::vec3(lightProjectionFarPlane * 0.8f));

	TRSTransformf lightOffsetTransform;
	lightOffsetTransform.SetTranslate(glm::vec3(0.0f, 0.0f, -0.4f) * constant::scale_lengths);


	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepthf(1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);


	glBindFramebuffer(GL_READ_FRAMEBUFFER, fbos[toU(FBO::Resolve)]);


	auto seconds_nb = 0.0f;
	std::array<GLuint64, toU(ElapsedTimeQuery::Count)> pass_elapsed_times;
	auto lastTime = std::chrono::high_resolution_clock::now();
	bool show_textures = true;
	bool show_cone_wireframe = false;

	bool show_logs = true;
	bool show_gui = true;
	bool shader_reload_failed = false;
	bool copy_elapsed_times = true;
	bool first_frame = true;
	bool show_basis = false;
	float basis_thickness_scale = 40.0f;
	float basis_length_scale = 400.0f;

	while (!glfwWindowShouldClose(window)) {
		auto const nowTime = std::chrono::high_resolution_clock::now();
		auto const deltaTimeUs = std::chrono::duration_cast<std::chrono::microseconds>(nowTime - lastTime);
		lastTime = nowTime;
		if (!are_lights_paused)
			seconds_nb += std::chrono::duration<decltype(seconds_nb)>(deltaTimeUs).count();

		auto& io = ImGui::GetIO();
		inputHandler.SetUICapture(io.WantCaptureMouse, io.WantCaptureKeyboard);

		glfwPollEvents();
		inputHandler.Advance();
		mCamera.Update(deltaTimeUs, inputHandler);

		if (inputHandler.GetKeycodeState(GLFW_KEY_R) & JUST_PRESSED) {
			shader_reload_failed = !program_manager.ReloadAllPrograms();
			if (shader_reload_failed)
				tinyfd_notifyPopup("Shader Program Reload Error",
				                   "An error occurred while reloading shader programs; see the logs for details.\n"
				                   "Rendering is suspended until the issue is solved. Once fixed, just reload the shaders again.",
				                   "error");
		}
		if (inputHandler.GetKeycodeState(GLFW_KEY_F3) & JUST_RELEASED)
			show_logs = !show_logs;
		if (inputHandler.GetKeycodeState(GLFW_KEY_F2) & JUST_RELEASED)
			show_gui = !show_gui;

		mWindowManager.NewImGuiFrame();

		if (!first_frame && show_gui && copy_elapsed_times) {
			// Copy all timings back from the GPU to the CPU.
			for (GLuint i = 0; i < pass_elapsed_times.size(); ++i) {
				glGetQueryObjectui64v(elapsed_time_queries[i], GL_QUERY_RESULT, pass_elapsed_times.data() + i);
			}
		}


		for (size_t i = 0; i < static_cast<size_t>(lights_nb); ++i) {
			lightTransforms[i].SetRotate(seconds_nb * 0.1f + i * 1.57f, glm::vec3(0.0f, 1.0f, 0.0f));
		}


		if (!shader_reload_failed) {
			//
			// Pass 1: Render scene into the g-buffer
			//
			utils::opengl::debug::beginDebugGroup("Fill G-buffer");
			glBeginQuery(GL_TIME_ELAPSED, elapsed_time_queries[toU(ElapsedTimeQuery::GbufferGeneration)]);

			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbos[toU(FBO::GBuffer)]);
			glViewport(0, 0, framebuffer_width, framebuffer_height);
			glClear(GL_DEPTH_BUFFER_BIT);
			// XXX: Is any other clearing needed?

			for (auto const& element : sponza_elements)
				element.render(mCamera.GetWorldToClipMatrix(), element.get_transform().GetMatrix(), fill_gbuffer_shader, set_uniforms);

			glEndQuery(GL_TIME_ELAPSED);
			utils::opengl::debug::endDebugGroup();



			//
			// Pass 2: Generate shadowmaps and accumulate lights' contribution
			//
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbos[toU(FBO::LightAccumulation)]);
			glViewport(0, 0, framebuffer_width, framebuffer_height);
			// XXX: Is any clearing needed?
			for (size_t i = 0; i < static_cast<size_t>(lights_nb); ++i) {
				auto const& lightTransform = lightTransforms[i];
				auto const light_view_matrix = lightOffsetTransform.GetMatrixInverse() * lightTransform.GetMatrixInverse();
				auto const light_world_matrix = glm::inverse(light_view_matrix) * coneScaleTransform.GetMatrix();
				auto const light_world_to_clip_matrix = lightProjection * light_view_matrix;

				//
				// Pass 2.1: Generate shadow map for light i
				//
				utils::opengl::debug::beginDebugGroup("Create shadow map " + std::to_string(i));
				glBeginQuery(GL_TIME_ELAPSED, elapsed_time_queries[toU(ElapsedTimeQuery::ShadowMap0Generation) + i]);

				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbos[toU(FBO::ShadowMap)]);
				glViewport(0, 0, constant::shadowmap_res_x, constant::shadowmap_res_y);
				// XXX: Is any clearing needed?

				for (auto const& element : sponza_elements)
					element.render(light_world_to_clip_matrix, element.get_transform().GetMatrix(), fill_shadowmap_shader, set_uniforms);

				glEndQuery(GL_TIME_ELAPSED);
				utils::opengl::debug::endDebugGroup();


				glCullFace(GL_FRONT);
				glEnable(GL_BLEND);
				glDepthFunc(GL_GREATER);
				glDepthMask(GL_FALSE);
				glBlendEquationSeparate(GL_FUNC_ADD, GL_MIN);
				glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ONE);
				//
				// Pass 2.2: Accumulate light i contribution
				utils::opengl::debug::beginDebugGroup("Accumulate light " + std::to_string(i));
				glBeginQuery(GL_TIME_ELAPSED, elapsed_time_queries[toU(ElapsedTimeQuery::Light0Accumulation) + i]);

				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbos[toU(FBO::LightAccumulation)]);
				glUseProgram(accumulate_lights_shader);
				glViewport(0, 0, framebuffer_width, framebuffer_height);
				// XXX: Is any clearing needed?

				auto const spotlight_set_uniforms = [framebuffer_width,framebuffer_height,this,&light_world_to_clip_matrix,&lightColors,&lightTransform,&i](GLuint program){
					glUniform2f(glGetUniformLocation(program, "inv_res"),
					            1.0f / static_cast<float>(framebuffer_width),
					            1.0f / static_cast<float>(framebuffer_height));
					glUniformMatrix4fv(glGetUniformLocation(program, "view_projection_inverse"), 1, GL_FALSE,
					                   glm::value_ptr(mCamera.GetClipToWorldMatrix()));
					glUniform3fv(glGetUniformLocation(program, "camera_position"), 1,
					                   glm::value_ptr(mCamera.mWorld.GetTranslation()));
					glUniformMatrix4fv(glGetUniformLocation(program, "shadow_view_projection"), 1, GL_FALSE,
					                   glm::value_ptr(light_world_to_clip_matrix));
					glUniform3fv(glGetUniformLocation(program, "light_color"), 1, glm::value_ptr(lightColors[i]));
					glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(lightTransform.GetTranslation()));
					glUniform3fv(glGetUniformLocation(program, "light_direction"), 1, glm::value_ptr(lightTransform.GetFront()));
					glUniform1f(glGetUniformLocation(program, "light_intensity"), constant::light_intensity);
					glUniform1f(glGetUniformLocation(program, "light_angle_falloff"), constant::light_angle_falloff);
					glUniform2f(glGetUniformLocation(program, "shadowmap_texel_size"),
					            1.0f / static_cast<float>(constant::shadowmap_res_x),
					            1.0f / static_cast<float>(constant::shadowmap_res_y));
				};

				bind_texture_with_sampler(GL_TEXTURE_2D, 0, accumulate_lights_shader, "depth_texture", textures[toU(Texture::DepthBuffer)], samplers[toU(Sampler::Nearest)]);
				bind_texture_with_sampler(GL_TEXTURE_2D, 1, accumulate_lights_shader, "normal_texture", textures[toU(Texture::GBufferWorldSpaceNormal)], samplers[toU(Sampler::Nearest)]);
				bind_texture_with_sampler(GL_TEXTURE_2D, 2, accumulate_lights_shader, "shadow_texture", textures[toU(Texture::ShadowMap)], samplers[toU(Sampler::Shadow)]);

				cone.render(mCamera.GetWorldToClipMatrix(), light_world_matrix,
				            accumulate_lights_shader, spotlight_set_uniforms);

				glBindSampler(2u, 0u);
				glBindSampler(1u, 0u);
				glBindSampler(0u, 0u);

				glEndQuery(GL_TIME_ELAPSED);
				utils::opengl::debug::endDebugGroup();

				glDepthMask(GL_TRUE);
				glDepthFunc(GL_LESS);
				glDisable(GL_BLEND);
				glCullFace(GL_BACK);
			}


			//
			// Pass 3: Compute final image using both the g-buffer and  the light accumulation buffer
			//
			utils::opengl::debug::beginDebugGroup("Resolve");
			glBeginQuery(GL_TIME_ELAPSED, elapsed_time_queries[toU(ElapsedTimeQuery::Resolve)]);

			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbos[toU(FBO::Resolve)]);
			glUseProgram(resolve_deferred_shader);
			glViewport(0, 0, framebuffer_width, framebuffer_height);
			// XXX: Is any clearing needed?

			bind_texture_with_sampler(GL_TEXTURE_2D, 0, resolve_deferred_shader, "diffuse_texture", textures[toU(Texture::GBufferDiffuse)], samplers[toU(Sampler::Nearest)]);
			bind_texture_with_sampler(GL_TEXTURE_2D, 1, resolve_deferred_shader, "specular_texture", textures[toU(Texture::GBufferSpecular)], samplers[toU(Sampler::Nearest)]);
			bind_texture_with_sampler(GL_TEXTURE_2D, 2, resolve_deferred_shader, "light_d_texture", textures[toU(Texture::LightDiffuseContribution)], samplers[toU(Sampler::Nearest)]);
			bind_texture_with_sampler(GL_TEXTURE_2D, 3, resolve_deferred_shader, "light_s_texture", textures[toU(Texture::LightSpecularContribution)], samplers[toU(Sampler::Nearest)]);

			bonobo::drawFullscreen();

			glBindSampler(3, 0u);
			glBindSampler(2, 0u);
			glBindSampler(1, 0u);
			glBindSampler(0, 0u);
			glUseProgram(0u);

			glEndQuery(GL_TIME_ELAPSED);
			utils::opengl::debug::endDebugGroup();
		}


		auto const show_debug_elements = show_cone_wireframe || show_basis;
		if (show_debug_elements) {
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbos[toU(FBO::FinalWithDepth)]);
		}


		//
		// Draw wireframe cones on top of the final image for debugging purposes
		//
		glBeginQuery(GL_TIME_ELAPSED, elapsed_time_queries[toU(ElapsedTimeQuery::ConeWireframe)]);
		if (show_cone_wireframe) {
			utils::opengl::debug::beginDebugGroup("Draw cone wireframe");

			glDisable(GL_CULL_FACE);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			for (size_t i = 0; i < lights_nb; ++i) {
				cone.render(mCamera.GetWorldToClipMatrix(),
				            lightTransforms[i].GetMatrix() * lightOffsetTransform.GetMatrix() * coneScaleTransform.GetMatrix(),
				            render_light_cones_shader, set_uniforms);
			}
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glEnable(GL_CULL_FACE);
			utils::opengl::debug::endDebugGroup();
		}
		glEndQuery(GL_TIME_ELAPSED);


		utils::opengl::debug::beginDebugGroup("Draw GUI");
		glBeginQuery(GL_TIME_ELAPSED, elapsed_time_queries[toU(ElapsedTimeQuery::GUI)]);

		//
		// Display 3D helpers
		//
		if (show_basis) {
			bonobo::renderBasis(basis_thickness_scale, basis_length_scale, mCamera.GetWorldToClipMatrix());
		}

		// If the basis and cone wireframe were not shown, FBO::Resolve
		// is still bound so there is no need to rebind it.
		if (show_debug_elements) {
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbos[toU(FBO::Resolve)]);
		}

		//
		// Output content of the g-buffer as well as of the shadowmap, for debugging purposes
		//
		if (show_textures) {
			bonobo::displayTexture({-0.95f, -0.95f}, {-0.55f, -0.55f}, textures[toU(Texture::GBufferDiffuse)],            samplers[toU(Sampler::Linear)], {0, 1, 2, -1}, glm::uvec2(framebuffer_width, framebuffer_height));
			bonobo::displayTexture({-0.45f, -0.95f}, {-0.05f, -0.55f}, textures[toU(Texture::GBufferSpecular)],           samplers[toU(Sampler::Linear)], {0, 1, 2, -1}, glm::uvec2(framebuffer_width, framebuffer_height));
			bonobo::displayTexture({ 0.05f, -0.95f}, { 0.45f, -0.55f}, textures[toU(Texture::GBufferWorldSpaceNormal)],   samplers[toU(Sampler::Linear)], {0, 1, 2, -1}, glm::uvec2(framebuffer_width, framebuffer_height));
			bonobo::displayTexture({ 0.55f, -0.95f}, { 0.95f, -0.55f}, textures[toU(Texture::DepthBuffer)],               samplers[toU(Sampler::Linear)], {0, 0, 0, -1}, glm::uvec2(framebuffer_width, framebuffer_height), true, mCamera.mNear, mCamera.mFar);
			bonobo::displayTexture({-0.95f,  0.55f}, {-0.55f,  0.95f}, textures[toU(Texture::ShadowMap)],                 samplers[toU(Sampler::Linear)], {0, 0, 0, -1}, glm::uvec2(framebuffer_width, framebuffer_height), true, lightProjectionNearPlane, lightProjectionFarPlane);
			bonobo::displayTexture({-0.45f,  0.55f}, {-0.05f,  0.95f}, textures[toU(Texture::LightDiffuseContribution)],  samplers[toU(Sampler::Linear)], {0, 1, 2, -1}, glm::uvec2(framebuffer_width, framebuffer_height));
			bonobo::displayTexture({ 0.05f,  0.55f}, { 0.45f,  0.95f}, textures[toU(Texture::LightSpecularContribution)], samplers[toU(Sampler::Linear)], {0, 1, 2, -1}, glm::uvec2(framebuffer_width, framebuffer_height));
		}

		//
		// Reset viewport back to normal
		//
		glViewport(0, 0, framebuffer_width, framebuffer_height);

		bool opened = ImGui::Begin("Render Time", nullptr, ImGuiWindowFlags_None);
		if (opened) {
			ImGui::Text("Frame CPU time: %.3f ms", std::chrono::duration<float, std::milli>(deltaTimeUs).count());

			ImGui::Checkbox("Copy elapsed times back to CPU", &copy_elapsed_times);

			if (ImGui::BeginTable("Pass durations", 2, ImGuiTableFlags_SizingFixedFit))
			{
				ImGui::TableSetupColumn("Pass");
				ImGui::TableSetupColumn("GPU time [ms]");
				ImGui::TableHeadersRow();

				ImGui::TableNextColumn();
				ImGui::Text("Gbuffer gen.");
				ImGui::TableNextColumn();
				ImGui::Text("%.3f", pass_elapsed_times[toU(ElapsedTimeQuery::GbufferGeneration)] / 1000000.0f);

				for (std::size_t i = 0; i < lights_nb; ++i) {
					ImGui::TableNextColumn();
					ImGui::Text("Light %zu", i);
					ImGui::TableNextColumn();
					ImGui::Text("");

					ImGui::TableNextColumn();
					ImGui::Text("  Shadow map");
					ImGui::TableNextColumn();
					ImGui::Text("%.3f", pass_elapsed_times[toU(ElapsedTimeQuery::ShadowMap0Generation) + i] / 1000000.0f);

					ImGui::TableNextColumn();
					ImGui::Text("  Light accumulation");
					ImGui::TableNextColumn();
					ImGui::Text("%.3f", pass_elapsed_times[toU(ElapsedTimeQuery::Light0Accumulation) + i] / 1000000.0f);
				}

				ImGui::TableNextColumn();
				ImGui::Text("Resolve");
				ImGui::TableNextColumn();
				ImGui::Text("%.3f", pass_elapsed_times[toU(ElapsedTimeQuery::Resolve)] / 1000000.0f);

				ImGui::TableNextColumn();
				ImGui::Text("Cone wireframe");
				ImGui::TableNextColumn();
				ImGui::Text("%.3f", pass_elapsed_times[toU(ElapsedTimeQuery::ConeWireframe)] / 1000000.0f);

				ImGui::TableNextColumn();
				ImGui::Text("GUI");
				ImGui::TableNextColumn();
				ImGui::Text("%.3f", pass_elapsed_times[toU(ElapsedTimeQuery::GUI)] / 1000000.0f);

				ImGui::TableNextColumn();
				ImGui::Text("Copy to framebuffer");
				ImGui::TableNextColumn();
				ImGui::Text("%.3f", pass_elapsed_times[toU(ElapsedTimeQuery::CopyToFramebuffer)] / 1000000.0f);

				ImGui::EndTable();
			}
		}
		ImGui::End();

		opened = ImGui::Begin("Scene Controls", nullptr, ImGuiWindowFlags_None);
		if (opened) {
			ImGui::Checkbox("Pause lights", &are_lights_paused);
			ImGui::SliderInt("Number of lights", &lights_nb, 1, static_cast<int>(constant::lights_nb));
			ImGui::Checkbox("Show textures", &show_textures);
			ImGui::Checkbox("Show light cones wireframe", &show_cone_wireframe);
			ImGui::Separator();
			ImGui::Checkbox("Show basis", &show_basis);
			ImGui::SliderFloat("Basis thickness scale", &basis_thickness_scale, 0.0f, 100.0f);
			ImGui::SliderFloat("Basis length scale", &basis_length_scale, 0.0f, 100.0f);
		}
		ImGui::End();

		if (show_logs)
			Log::View::Render();
		mWindowManager.RenderImGuiFrame(show_gui);

		glEndQuery(GL_TIME_ELAPSED);
		utils::opengl::debug::endDebugGroup();

		//
		// Blit the result back to the default framebuffer.
		//
		utils::opengl::debug::beginDebugGroup("Copy to default framebuffer");
		glBeginQuery(GL_TIME_ELAPSED, elapsed_time_queries[toU(ElapsedTimeQuery::CopyToFramebuffer)]);

		// FBO::Resolve has already been bound to GL_READ_FRAMEBUFFER before rendering the first frame,
		// as no other frame buffer gets bound to it.
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0u);
		glBlitFramebuffer(0, 0, framebuffer_width, framebuffer_height, 0, 0, framebuffer_width, framebuffer_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

		glEndQuery(GL_TIME_ELAPSED);
		utils::opengl::debug::endDebugGroup();

		glfwSwapBuffers(window);

		first_frame = false;
	}

	glDeleteQueries(static_cast<GLsizei>(elapsed_time_queries.size()), elapsed_time_queries.data());
	glDeleteSamplers(static_cast<GLsizei>(samplers.size()), samplers.data());
	glDeleteFramebuffers(static_cast<GLsizei>(fbos.size()), fbos.data());
	glDeleteTextures(static_cast<GLsizei>(textures.size()), textures.data());

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
	std::setlocale(LC_ALL, "");

	Bonobo framework;

	try {
		edan35::Assignment2 assignment2(framework.GetWindowManager());
		assignment2.run();
	} catch (std::runtime_error const& e) {
		LogError(e.what());
	}
}

namespace
{
Textures createTextures(GLsizei framebuffer_width, GLsizei framebuffer_height)
{
	Textures textures;
	glGenTextures(static_cast<GLsizei>(textures.size()), textures.data());

	glBindTexture(GL_TEXTURE_2D, textures[toU(Texture::DepthBuffer)]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, framebuffer_width, framebuffer_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	utils::opengl::debug::nameObject(GL_TEXTURE, textures[toU(Texture::DepthBuffer)], "Depth buffer");

	glBindTexture(GL_TEXTURE_2D, textures[toU(Texture::ShadowMap)]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, constant::shadowmap_res_x, constant::shadowmap_res_y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	utils::opengl::debug::nameObject(GL_TEXTURE, textures[toU(Texture::ShadowMap)], "Shadow map");

	glBindTexture(GL_TEXTURE_2D, textures[toU(Texture::GBufferDiffuse)]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, framebuffer_width, framebuffer_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	utils::opengl::debug::nameObject(GL_TEXTURE, textures[toU(Texture::GBufferDiffuse)], "GBuffer diffuse");

	glBindTexture(GL_TEXTURE_2D, textures[toU(Texture::GBufferSpecular)]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, framebuffer_width, framebuffer_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	utils::opengl::debug::nameObject(GL_TEXTURE, textures[toU(Texture::GBufferSpecular)], "GBuffer specular");

	glBindTexture(GL_TEXTURE_2D, textures[toU(Texture::GBufferWorldSpaceNormal)]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, framebuffer_width, framebuffer_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	utils::opengl::debug::nameObject(GL_TEXTURE, textures[toU(Texture::GBufferWorldSpaceNormal)], "GBuffer normals");

	glBindTexture(GL_TEXTURE_2D, textures[toU(Texture::LightDiffuseContribution)]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, framebuffer_width, framebuffer_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	utils::opengl::debug::nameObject(GL_TEXTURE, textures[toU(Texture::LightDiffuseContribution)], "Light diffuse contribution");

	glBindTexture(GL_TEXTURE_2D, textures[toU(Texture::LightSpecularContribution)]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, framebuffer_width, framebuffer_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	utils::opengl::debug::nameObject(GL_TEXTURE, textures[toU(Texture::LightSpecularContribution)], "Light specular contribution");

	glBindTexture(GL_TEXTURE_2D, textures[toU(Texture::Result)]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, framebuffer_width, framebuffer_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	utils::opengl::debug::nameObject(GL_TEXTURE, textures[toU(Texture::Result)], "Final result");

	glBindTexture(GL_TEXTURE_2D, 0u);
	return textures;
}

Samplers createSamplers()
{
	Samplers samplers;
	glGenSamplers(static_cast<GLsizei>(samplers.size()), samplers.data());

	// For sampling 2-D textures without interpolation.
	glSamplerParameteri(samplers[toU(Sampler::Nearest)], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glSamplerParameteri(samplers[toU(Sampler::Nearest)], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glSamplerParameteri(samplers[toU(Sampler::Nearest)], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(samplers[toU(Sampler::Nearest)], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	utils::opengl::debug::nameObject(GL_SAMPLER, samplers[toU(Sampler::Nearest)], "Nearest");

	// For sampling 2-D textures without mipmaps.
	glSamplerParameteri(samplers[toU(Sampler::Linear)], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glSamplerParameteri(samplers[toU(Sampler::Linear)], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(samplers[toU(Sampler::Linear)], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(samplers[toU(Sampler::Linear)], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	utils::opengl::debug::nameObject(GL_SAMPLER, samplers[toU(Sampler::Linear)], "Linear");

	// For sampling 2-D textures with mipmaps.
	glSamplerParameteri(samplers[toU(Sampler::Mipmaps)], GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glSamplerParameteri(samplers[toU(Sampler::Mipmaps)], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(samplers[toU(Sampler::Mipmaps)], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(samplers[toU(Sampler::Mipmaps)], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	utils::opengl::debug::nameObject(GL_SAMPLER, samplers[toU(Sampler::Mipmaps)], "Mimaps");

	// For sampling 2-D shadow maps
	glSamplerParameteri(samplers[toU(Sampler::Shadow)], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glSamplerParameteri(samplers[toU(Sampler::Shadow)], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glSamplerParameteri(samplers[toU(Sampler::Shadow)], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(samplers[toU(Sampler::Shadow)], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(samplers[toU(Sampler::Shadow)], GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glSamplerParameteri(samplers[toU(Sampler::Shadow)], GL_TEXTURE_COMPARE_FUNC, GL_LESS);
	utils::opengl::debug::nameObject(GL_SAMPLER, samplers[toU(Sampler::Shadow)], "Shadow");

	return samplers;
}

FBOs createFramebufferObjects(Textures const& textures)
{
	auto const validate_fbo = [](std::string const& fbo_name){
		auto const status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status == GL_FRAMEBUFFER_COMPLETE)
			return;

		LogError("Framebuffer \"%s\" is not complete: check the logs for additional information.", fbo_name.data());
	};

	FBOs fbos;
	glGenFramebuffers(static_cast<GLsizei>(fbos.size()), fbos.data());

	glBindFramebuffer(GL_FRAMEBUFFER, fbos[toU(FBO::GBuffer)]);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[toU(Texture::GBufferDiffuse)], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, textures[toU(Texture::GBufferSpecular)], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, textures[toU(Texture::GBufferWorldSpaceNormal)], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, textures[toU(Texture::DepthBuffer)], 0);
	glReadBuffer(GL_NONE); // Disable reading back from the colour attachments, as unnecessary in this assignment.
	// Configure the mapping from fragment shader outputs to colour attachments.
	std::array<GLenum, 3> const gbuffer_draws = {
		GL_COLOR_ATTACHMENT0, // The fragment shader output at location 0 will be written to colour attachment 0 (i.e. the diffuse texture).
		GL_COLOR_ATTACHMENT1, // The fragment shader output at location 1 will be written to colour attachment 1 (i.e. the specular texture).
		GL_COLOR_ATTACHMENT2  // The fragment shader output at location 2 will be written to colour attachment 2 (i.e. the normal texture).
	};
	glDrawBuffers(static_cast<GLsizei>(gbuffer_draws.size()), gbuffer_draws.data());
	validate_fbo("GBuffer");
	utils::opengl::debug::nameObject(GL_FRAMEBUFFER, fbos[toU(FBO::GBuffer)], "GBuffer");

	glBindFramebuffer(GL_FRAMEBUFFER, fbos[toU(FBO::ShadowMap)]);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, textures[toU(Texture::ShadowMap)], 0);
	validate_fbo("Shadow map generation");
	utils::opengl::debug::nameObject(GL_FRAMEBUFFER, fbos[toU(FBO::ShadowMap)], "Shadow map generation");

	glBindFramebuffer(GL_FRAMEBUFFER, fbos[toU(FBO::LightAccumulation)]);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[toU(Texture::LightDiffuseContribution)], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, textures[toU(Texture::LightSpecularContribution)], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, textures[toU(Texture::DepthBuffer)], 0);
	glReadBuffer(GL_NONE); // Disable reading back from the colour attachments, as unnecessary in this assignment.
	// Configure the mapping from fragment shader outputs to colour attachments.
	std::array<GLenum, 2> const light_accumulation_draws = {
		GL_COLOR_ATTACHMENT0, // The fragment shader output at location 0 will be written to colour attachment 0 (i.e. the light diffuse contribution texture).
		GL_COLOR_ATTACHMENT1  // The fragment shader output at location 1 will be written to colour attachment 1 (i.e. the light specular contribution texture).
	};
	glDrawBuffers(static_cast<GLsizei>(light_accumulation_draws.size()), light_accumulation_draws.data());
	validate_fbo("Light accumulation");
	utils::opengl::debug::nameObject(GL_FRAMEBUFFER, fbos[toU(FBO::LightAccumulation)], "Light acccumulation");

	glBindFramebuffer(GL_FRAMEBUFFER, fbos[toU(FBO::Resolve)]);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[toU(Texture::Result)], 0);
	glReadBuffer(GL_COLOR_ATTACHMENT0); // Colour attachment result 0 (i.e. the rendering result texture) will be blitted to the screen.
	glDrawBuffer(GL_COLOR_ATTACHMENT0); // The fragment shader output at location 0 will be written to colour attachment 0 (i.e. the rendering result texture).
	validate_fbo("Resolve");
	utils::opengl::debug::nameObject(GL_FRAMEBUFFER, fbos[toU(FBO::Resolve)], "Resolve");

	glBindFramebuffer(GL_FRAMEBUFFER, fbos[toU(FBO::FinalWithDepth)]);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[toU(Texture::Result)], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, textures[toU(Texture::DepthBuffer)], 0);
	glReadBuffer(GL_NONE); // Disable reading back from the colour attachments, as unnecessary in this assignment.
	glDrawBuffer(GL_COLOR_ATTACHMENT0); // The fragment shader output at location 0 will be written to colour attachment 0 (i.e. the rendering result texture).
	validate_fbo("Final with depth");
	utils::opengl::debug::nameObject(GL_FRAMEBUFFER, fbos[toU(FBO::FinalWithDepth)], "Cone wireframe");

	glBindFramebuffer(GL_FRAMEBUFFER, 0u);
	return fbos;
}

ElapsedTimeQueries createElapsedTimeQueries()
{
	ElapsedTimeQueries queries;
	glGenQueries(static_cast<GLsizei>(queries.size()), queries.data());

	if (utils::opengl::debug::isSupported())
	{
		// Queries (like any other OpenGL object) need to have been used at least
		// once to ensure their resources have been allocated so we can call
		// `glObjectLabel()` on them.
		auto const register_query = [](GLuint const query) {
			glBeginQuery(GL_TIME_ELAPSED, query);
			glEndQuery(GL_TIME_ELAPSED);
		};

		register_query(queries[toU(ElapsedTimeQuery::GbufferGeneration)]);
		utils::opengl::debug::nameObject(GL_QUERY, queries[toU(ElapsedTimeQuery::GbufferGeneration)], "GBuffer generation");

		for (size_t i = 0; i < constant::lights_nb; ++i)
		{
			register_query(queries[toU(ElapsedTimeQuery::ShadowMap0Generation) + i]);
			utils::opengl::debug::nameObject(GL_QUERY, queries[toU(ElapsedTimeQuery::ShadowMap0Generation) + i], "Shadow map " + std::to_string(i) + " generation");

			register_query(queries[toU(ElapsedTimeQuery::Light0Accumulation) + i]);
			utils::opengl::debug::nameObject(GL_QUERY, queries[toU(ElapsedTimeQuery::Light0Accumulation) + i], "Light" + std::to_string(i) + " accumulation");
		}

		register_query(queries[toU(ElapsedTimeQuery::Resolve)]);
		utils::opengl::debug::nameObject(GL_QUERY, queries[toU(ElapsedTimeQuery::Resolve)], "Resolve");

		register_query(queries[toU(ElapsedTimeQuery::ConeWireframe)]);
		utils::opengl::debug::nameObject(GL_QUERY, queries[toU(ElapsedTimeQuery::ConeWireframe)], "Cone wireframe");

		register_query(queries[toU(ElapsedTimeQuery::GUI)]);
		utils::opengl::debug::nameObject(GL_QUERY, queries[toU(ElapsedTimeQuery::GUI)], "GUI");
	}

	return queries;
}

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
		utils::opengl::debug::nameObject(GL_VERTEX_ARRAY, cone.vao, "Cone VAO");

		glGenBuffers(1, &cone.bo);
		assert(cone.bo != 0u);
		glBindBuffer(GL_ARRAY_BUFFER, cone.bo);
		glBufferData(GL_ARRAY_BUFFER, cone.vertices_nb * 3 * sizeof(float), vertexArrayData, GL_STATIC_DRAW);
		utils::opengl::debug::nameObject(GL_BUFFER, cone.bo, "Cone VBO");

		glVertexAttribPointer(static_cast<int>(bonobo::shader_bindings::vertices), 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid const*>(0x0));
		glEnableVertexAttribArray(static_cast<int>(bonobo::shader_bindings::vertices));

		glBindBuffer(GL_ARRAY_BUFFER, 0u);
	}
	glBindVertexArray(0u);

	return cone;
}
} // namespace
