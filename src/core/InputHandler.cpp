#include "InputHandler.h"
#include "Log.h"

/*----------------------------------------------------------------------------*/

InputHandler::InputHandler()
{
	for (auto& mousePosition : mMousePositionSwitched)
	{
		mousePosition = mMousePosition;
	}
}

void InputHandler::Advance()
{
	mTick++;
}

void InputHandler::DownEvent(std::unordered_map<size_t, IState> &map, size_t loc)
{
	auto sc = map.find(loc);
	if (sc == map.end())
		map[static_cast<size_t>(loc)] = IState();
	map[loc].mIsDown = true;
	map[loc].mDownTick = mTick;
}

void InputHandler::UpEvent(std::unordered_map<size_t, IState> &map, size_t loc)
{
	auto sc = map.find(loc);
	if (sc == map.end())
		map[static_cast<size_t>(loc)] = IState();
	map[loc].mIsDown = false;
	map[loc].mUpTick = mTick;
}

void InputHandler::FeedKeyboard(int key, int scancode, int action)
{
	switch (action)
	{
		case GLFW_PRESS:
			DownEvent(mScancodeMap, static_cast<size_t>(scancode));
			DownEvent(mKeycodeMap, static_cast<size_t>(key));
			break;
		case GLFW_RELEASE:
			UpEvent(mScancodeMap, static_cast<size_t>(scancode));
			UpEvent(mKeycodeMap, static_cast<size_t>(key));
			break;
		default:
			break;
	}
}

void InputHandler::FeedMouseMotion(glm::vec2 const& position)
{
  mMousePosition = position;
}

void InputHandler::FeedMouseButtons(int button, int action)
{
	switch (action)
	{
		case GLFW_PRESS:
			DownEvent(mMouseMap, static_cast<size_t>(button));
			mMousePositionSwitched[button] = mMousePosition;
			break;
		case GLFW_RELEASE:
			UpEvent(mMouseMap, static_cast<size_t>(button));
			mMousePositionSwitched[button] = mMousePosition;
			break;
		default:
			break;
	}
}

std::uint32_t InputHandler::GetState(std::unordered_map<size_t, IState> &map, size_t loc)
{
	auto sc = map.find(loc);
	if (sc == map.end())
		return RELEASED;
	std::uint32_t s = map[static_cast<std::uint32_t>(loc)].mIsDown ? PRESSED : RELEASED;
	s |= mTick-1 == map[loc].mDownTick ? JUST_PRESSED : 0;
	s |= mTick-1 == map[loc].mUpTick ? JUST_RELEASED : 0;
	return s;
}

std::uint32_t InputHandler::GetScancodeState(int scancode)
{
	return GetState(mScancodeMap, static_cast<size_t>(scancode));
}

std::uint32_t InputHandler::GetKeycodeState(int  key)
{
	return GetState(mKeycodeMap, static_cast<size_t>(key));
}

std::uint32_t InputHandler::GetMouseState(std::uint32_t button)
{
	return GetState(mMouseMap, static_cast<size_t>(button));
}

glm::vec2 InputHandler::GetMousePositionAtStateShift(std::uint32_t button)
{
	return mMousePositionSwitched[button];
}

glm::vec2 InputHandler::GetMousePosition()
{
	return mMousePosition;
}

bool InputHandler::IsMouseCapturedByUI() const
{
	return mMouseCapturedByUI;
}

bool InputHandler::IsKeyboardCapturedByUI() const
{
	return mKeyboardCapturedByUI;
}

void InputHandler::SetUICapture(bool mouseCapture, bool keyboardCapture)
{
	mMouseCapturedByUI = mouseCapture;
	mKeyboardCapturedByUI = keyboardCapture;
}
