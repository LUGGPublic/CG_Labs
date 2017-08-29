#pragma once


class InputHandler;
class Window;

namespace edaf80
{
	//! \brief Wrapper class for Assignment 1
	class Assignment1 {
	public:
		//! \brief Default constructor.
		//!
		//! It will initialise various modules of bonobo and retrieve a
		//! window to draw to.
		Assignment1();

		//! \brief Default destructor.
		//!
		//! It will release the bonobo modules initialised by the
		//! constructor, as well as the window.
		~Assignment1();

		//! \brief Contains the logic of the assignment, along with the
		//! render loop.
		void run();

	private:
		InputHandler *inputHandler;
		Window       *window;
	};
}
