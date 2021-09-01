#pragma once

#include "core/InputHandler.h"
#include "core/FPSCamera.h"
#include "core/WindowManager.hpp"


class Window;


namespace edaf80
{
	//! \brief Wrapper class for Assignment 3
	class Assignment3 {
	public:
		//! \brief Default constructor.
		//!
		//! It will initialise various modules of bonobo and retrieve a
		//! window to draw to.
		Assignment3(WindowManager& windowManager);

		//! \brief Default destructor.
		//!
		//! It will release the bonobo modules initialised by the
		//! constructor, as well as the window.
		~Assignment3();

		//! \brief Contains the logic of the assignment, along with the
		//! render loop.
		void run();

	private:
		FPSCameraf     mCamera;
		InputHandler   inputHandler;
		WindowManager& mWindowManager;
		GLFWwindow*    window;
	};
}
