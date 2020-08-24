#pragma once

#include <cstdint>
#include <unordered_map>

#define GLFW_INCLUDE_NONE
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
		std::uint64_t mDownTick{std::numeric_limits<std::uint64_t>::max()};
		std::uint64_t mUpTick{std::numeric_limits<std::uint64_t>::max()};
		bool mIsDown{false};
	};

public:
	InputHandler();

public:
	void FeedKeyboard(int key, int scancode, int action, int mods);
	void FeedMouseButtons(int button, int action, int mods);
	void FeedMouseMotion(glm::vec2 const& position);
	void Advance();
	std::uint32_t GetScancodeState(int scancode);
	std::uint32_t GetKeycodeState(int key);
	std::uint32_t GetMouseState(std::uint32_t button);
	glm::vec2 GetMousePositionAtStateShift(std::uint32_t button);
	glm::vec2 GetMousePosition();
	bool IsMouseCapturedByUI() const;
	bool IsKeyboardCapturedByUI() const;
	void SetUICapture(bool mouseCapture, bool keyboardCapture);

private:
	void DownEvent(std::unordered_map<size_t, IState> &map, size_t loc);
	void DownModEvent(std::unordered_map<size_t, IState> &map, std::uint32_t mods);
	void UpEvent(std::unordered_map<size_t, IState> &map, size_t loc);
	void UpModEvent(std::unordered_map<size_t, IState> &map, std::uint32_t mods);

	std::uint32_t GetState(std::unordered_map<size_t, IState> &map, size_t loc);

	std::unordered_map<size_t, IState> mScancodeMap;
	std::unordered_map<size_t, IState> mKeycodeMap;
	std::unordered_map<size_t, IState> mMouseMap;

	glm::vec2 mMousePosition;
	glm::vec2 mMousePositionSwitched[MAX_MOUSE_BUTTONS];

	bool mMouseCapturedByUI;
	bool mKeyboardCapturedByUI;

	std::uint64_t mTick;

};

