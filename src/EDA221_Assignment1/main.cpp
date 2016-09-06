#include "SolarSystem.h"

#include "core/Bonobo.h"


int main()
{
	Bonobo::Init();
	{
		SolarSystem assignment1;
		assignment1.run();
	}
	Bonobo::Destroy();
}

