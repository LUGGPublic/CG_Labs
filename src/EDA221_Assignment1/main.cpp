#include "SolarSystem.h"

#include "core/Bonobo.h"
#include "core/Log.h"

#include <stdexcept>


int main()
{
	Bonobo::Init();
	try {
		SolarSystem assignment1;
		assignment1.run();
	} catch (std::runtime_error const& e) {
		LogError(e.what());
	}
	Bonobo::Destroy();
}

