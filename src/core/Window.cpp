#include <unordered_map>

#include "external/glad/glad.h"
#include <GLFW/glfw3.h>
#include <imgui.h>

#include "InputHandler.h"
#include "Log.h"
#include "opengl.hpp"
#include "Window.h"

#include "external/imgui_impl_glfw_gl3.h"

static std::unordered_map<std::string, Window *> *windowMap = nullptr;
static int default_opengl_major_version = 4;
static int default_opengl_minor_version = 1;

void Window::ErrorCallback(int error, char const* description)
{
  if (error == 65545 || error == 65543)
    LogInfo("Couldn't create an OpenGL %d.%d context.\n", default_opengl_major_version, default_opengl_minor_version);
  else
    LogError("GLFW error %d was thrown:\n\t%s\n", error, description);
}

void Window::KeyCallback( GLFWwindow* window, int key, int scancode
                            , int action, int mods)
{
  Window* instance = static_cast<Window*>(glfwGetWindowUserPointer(window));
  instance->mInputHandler->FeedKeyboard(key, scancode, action, mods);

#ifdef _WIN32
  bool should_close = (key == GLFW_KEY_F4) && (mods == GLFW_MOD_ALT);
#elif defined __APPLE__
  bool should_close = (key == GLFW_KEY_Q) && (mods == GLFW_MOD_SUPER);
#elif defined __linux__
  bool should_close = (key == GLFW_KEY_Q) && (mods == GLFW_MOD_CONTROL);
#else
  bool should_close = false;
#endif
  should_close |= (key == GLFW_KEY_ESCAPE);
  if (should_close)
    glfwSetWindowShouldClose(window, true);

  ImGui_ImplGlfwGL3_KeyCallback(window, key, scancode, action, mods);
}

void Window::MouseCallback( GLFWwindow* window, int button, int action
                              , int mods
                              )
{
  Window* instance = static_cast<Window*>(glfwGetWindowUserPointer(window));
  instance->mInputHandler->FeedMouseButtons(button, action, mods);
}

void Window::CursorCallback(GLFWwindow* window, double x, double y)
{
  Window* instance = static_cast<Window*>(glfwGetWindowUserPointer(window));
  instance->mInputHandler->FeedMouseMotion(glm::vec2(x, y));
}

void Window::FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	Window* instance = static_cast<Window*>(glfwGetWindowUserPointer(window));
	instance->mWidth = static_cast<unsigned int>(width);
	instance->mHeight = static_cast<unsigned int>(height);
	if (instance->mCamera != nullptr)
		instance->mCamera->SetAspect(static_cast<float>(width) / static_cast<float>(height));
}


Window *Window::Create(std::string mTitle, unsigned int w, unsigned int h, unsigned int msaa, bool fullscreen, bool resizable_, SwapStrategy swap)
{
	bool ok = Param(w > 0 && h > 0) ||
		Param(!mTitle.empty());
	if (!ok) {
		LogWarning("No window created! (%s)", mTitle.c_str());
		return nullptr;
	}
	auto elem = windowMap->find(mTitle);
	if (elem != windowMap->end()) {
		LogWarning("A window named %s already exists", mTitle.c_str());
		return nullptr;
	}
	Window *window = new Window(mTitle, w, h, msaa, fullscreen, resizable_, swap);
	if (window->mWindowGLFW == nullptr) {
		delete window;
		return nullptr;
	}
	(*windowMap)[mTitle] = window;
	return window;
}

bool Window::Destroy(Window *window)
{
	auto elem = windowMap->find(window->GetTitle());
	if (elem == windowMap->end()) {
		LogWarning("This window isn't mine to destroy");
		return false;
	}
	windowMap->erase(window->GetTitle());
	delete window;
	return true;
}

std::string Window::GetTitle() const
{
	return mTitle;
}

Window::Window(std::string mTitle_, unsigned w_, unsigned h_, unsigned int msaa_, bool fullscreen_, bool resizable_, SwapStrategy swap_) :
	mTitle(mTitle_), mWidth(w_), mHeight(h_), mMSAA(msaa_), mFullscreen(fullscreen_), mResizable(resizable_), mSwap(swap_), mWindowGLFW(nullptr), mInputHandler(nullptr), mCamera(nullptr)
{
	Show();
}

Window::~Window()
{
}

bool Window::Show()
{
	if (mWindowGLFW == nullptr) {
#ifdef __APPLE__
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, default_opengl_major_version);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, default_opengl_minor_version);

		glfwWindowHint(GLFW_RESIZABLE, mResizable ? GLFW_TRUE : GLFW_FALSE);
		glfwWindowHint(GLFW_SAMPLES, static_cast<int>(mMSAA));

		GLFWmonitor* const monitor = mFullscreen ? glfwGetPrimaryMonitor()
                                                 : nullptr;
		mWindowGLFW = glfwCreateWindow(static_cast<int>(mWidth), static_cast<int>(mHeight)
                                ,mTitle.c_str() ,monitor, nullptr);

		if (mWindowGLFW == nullptr)
			return false;
	}

	int width, height;
	glfwGetFramebufferSize(mWindowGLFW, &width, &height);
	mWidth = static_cast<unsigned int>(width);
	mHeight = static_cast<unsigned int>(height);

	glfwMakeContextCurrent(mWindowGLFW);

	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
		LogError("[GLAD]: Failed to initialise OpenGL context.");
		glfwDestroyWindow(mWindowGLFW);
		mWindowGLFW = nullptr;
		return false;
	}

	ImGui_ImplGlfwGL3_Init(mWindowGLFW, false);

	glfwSetKeyCallback(mWindowGLFW, Window::KeyCallback);
	glfwSetInputMode(mWindowGLFW, GLFW_STICKY_KEYS, 1);
	glfwSetMouseButtonCallback(mWindowGLFW, Window::MouseCallback);
	glfwSetCursorPosCallback(mWindowGLFW, Window::CursorCallback);
	glfwSetFramebufferSizeCallback(mWindowGLFW, Window::FramebufferSizeCallback);
	glfwSetWindowUserPointer(mWindowGLFW, static_cast<void*>(this));

	glfwSetScrollCallback(mWindowGLFW, ImGui_ImplGlfwGL3_ScrollCallback);
	glfwSetCharCallback(mWindowGLFW, ImGui_ImplGlfwGL3_CharCallback);

	GLint major_version = 0, minor_version = 0, context_flags = 0, profile_mask = 0;
	glGetIntegerv(GL_MAJOR_VERSION, &major_version);
	glGetIntegerv(GL_MINOR_VERSION, &minor_version);
	glGetIntegerv(GL_CONTEXT_FLAGS, &context_flags);
	glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &profile_mask);
	LogInfo("Using OpenGL %d.%d with context options: profile=%s, debug=%s, forward compatible=%s.", major_version, minor_version
	       , (profile_mask & GL_CONTEXT_CORE_PROFILE_BIT) ? "core" : (profile_mask & GL_CONTEXT_COMPATIBILITY_PROFILE_BIT) ? "compatibility" : "unknown"
	       , (context_flags & GL_CONTEXT_FLAG_DEBUG_BIT) ? "true" : "false"
	       , (context_flags & GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT) ? "true" : "false"
	       );

	if ((major_version >= 4 && minor_version >= 3) || GLAD_GL_KHR_debug)
	{
#if DEBUG_LEVEL >= 2
		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(utils::opengl::debug::opengl_error_callback, nullptr);
#endif
#if DEBUG_LEVEL == 2
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_FALSE);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_MEDIUM, 0, nullptr, GL_TRUE);
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH, 0, nullptr, GL_TRUE);
#elif DEBUG_LEVEL == 3
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
#endif
	}
	else
	{
		LogInfo("DebugCallback is not core in OpenGL %d.%d, and sadly the GL_KHR_DEBUG extension is not available either.", major_version, minor_version);
	}

	glfwSwapInterval(mSwap);
	// TODO: Reinitiate renderer
	return true;
}

void Window::SetFullscreen(bool state)
{
	if (mFullscreen == state)
		return;
	if (mWindowGLFW)
		glfwDestroyWindow(mWindowGLFW);
	mFullscreen = state;
	if (!Show()) {
		LogError("Failed to change fullscreen state: reverting back.");
		mFullscreen = !state;
		Show();
	}
}

void Window::Init()
{
	glfwSetErrorCallback(Window::ErrorCallback);
	glfwInit();

	if (windowMap != nullptr)
		Window::Destroy();
	windowMap = new std::unordered_map<std::string, Window *>();
}

void Window::Destroy()
{
	if (windowMap == nullptr)
		return;
	for (auto it = windowMap->begin(); it != windowMap->end(); ++it)
		delete it->second;
	windowMap->clear();
	delete windowMap;
	windowMap = nullptr;
	glfwTerminate();
}

void Window::Swap() const
{
	glfwSwapBuffers(mWindowGLFW);
}

glm::ivec2 Window::GetDimensions() const
{
	return glm::ivec2(mWidth, mHeight);
}

GLFWwindow *Window::GetGLFW_Window() const
{
	return mWindowGLFW;
}

void Window::SetInputHandler(InputHandler *inputHandler)
{
	mInputHandler = inputHandler;
}

void Window::SetCamera(FPSCameraf *camera)
{
	mCamera = camera;
}
