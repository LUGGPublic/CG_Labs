#include "Bonobo.h"
#include "Log.h"

void Bonobo::Init()
{
	Log::Init();
	LogInfo("Running Bonobo v0.2");

	LogInfo("Initiating window management system...");

	LogInfo("Done");
}

void Bonobo::Destroy()
{
	Log::Destroy();
}

