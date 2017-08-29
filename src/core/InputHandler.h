#pragma once

#include "Types.h"

#include <unordered_map>

#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>

/*----------------------------------------------------------------------------*/

#define PRESSED						(1 << 0)
#define RELEASED					(1 << 1)
#define JUST_PRESSED				(1 << 2)
#define JUST_RELEASED				(1 << 3)

#define MAX_MOUSE_BUTTONS			8

class InputHandler
{
public:
	struct IState {
		IState() : mDownTick(-1), mUpTick(-1), mIsDown(false) {}
		u64		mDownTick;
		u64		mUpTick;
		bool	mIsDown;
	};

public:
	InputHandler();

public:
	void FeedKeyboard(int key, int scancode, int action, int mods);
	void FeedMouseButtons(int button, int action, int mods);
	void FeedMouseMotion(glm::vec2 const& position);
	void Advance();
	u32 GetScancodeState(int scancode);
	u32 GetKeycodeState(int key);
	u32 GetMouseState(u32 button);
	glm::vec2 GetMousePositionAtStateShift(u32 button);
	glm::vec2 GetMousePosition();
	bool IsMouseCapturedByUI() const;
	bool IsKeyboardCapturedByUI() const;
	void SetUICapture(bool mouseCapture, bool keyboardCapture);

private:
	void DownEvent(std::unordered_map<size_t, IState> &map, size_t loc);
	void DownModEvent(std::unordered_map<size_t, IState> &map, u32 mods);
	void UpEvent(std::unordered_map<size_t, IState> &map, size_t loc);
	void UpModEvent(std::unordered_map<size_t, IState> &map, u32 mods);

	u32 GetState(std::unordered_map<size_t, IState> &map, size_t loc);

	std::unordered_map<size_t, IState> mScancodeMap;
	std::unordered_map<size_t, IState> mKeycodeMap;
	std::unordered_map<size_t, IState> mMouseMap;

	glm::vec2 mMousePosition;
	glm::vec2 mMousePositionSwitched[MAX_MOUSE_BUTTONS];

	bool mMouseCapturedByUI;
	bool mKeyboardCapturedByUI;

	u64 mTick;

};

