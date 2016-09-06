#include "Bonobo.h"
#include "Log.h"
#include "Window.h"

void Bonobo::Init()
{
	Log::Init();
	LogInfo("Running Bonobo v0.2");

	LogInfo("Initiating window management system...");
	Window::Init();

	LogInfo("Done");
}

void Bonobo::Destroy()
{
	Window::Destroy();
	Log::Destroy();
}

