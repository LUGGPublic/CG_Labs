#pragma once


class InputHandler;
class Window;


class SolarSystem {
public:
	SolarSystem();
	~SolarSystem();
	void run();

private:
	InputHandler *inputHandler;
	Window       *window;
};

