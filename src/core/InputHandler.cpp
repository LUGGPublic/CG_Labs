#include "InputHandler.h"
#include "Log.h"

/*----------------------------------------------------------------------------*/

InputHandler::InputHandler()
{
	mMouseCapturedByUI = false;
	mKeyboardCapturedByUI = false;
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

void InputHandler::DownModEvent(std::unordered_map<size_t, IState> &map, u32 mods)
{
	for (u32 i = 1u; mods != 0; i <<= 1) {
		if ((mods & i) == 0)
			continue;

		InputHandler::DownEvent(map, static_cast<size_t>(i));
		mods &= ~i;
	}
}

void InputHandler::UpEvent(std::unordered_map<size_t, IState> &map, size_t loc)
{
	auto sc = map.find(loc);
	if (sc == map.end())
		map[static_cast<size_t>(loc)] = IState();
	map[loc].mIsDown = false;
	map[loc].mUpTick = mTick;
}

void InputHandler::UpModEvent(std::unordered_map<size_t, IState> &map, u32 mods)
{
	for (u32 i = 1u; mods != 0; i <<= 1) {
		if ((mods & i) == 0)
			continue;

		InputHandler::UpEvent(map, static_cast<size_t>(i));
		mods &= ~i;
	}
}

void InputHandler::FeedKeyboard(int key, int scancode, int action, int mods)
{
	switch (action)
	{
		case GLFW_PRESS:
			DownEvent(mScancodeMap, static_cast<size_t>(scancode));
			DownModEvent(mScancodeMap, static_cast<u32>(mods));
			DownEvent(mKeycodeMap, static_cast<size_t>(key));
			DownModEvent(mKeycodeMap, static_cast<u32>(mods));
			break;
		case GLFW_RELEASE:
			UpEvent(mScancodeMap, static_cast<size_t>(scancode));
			UpModEvent(mScancodeMap, static_cast<u32>(mods));
			UpEvent(mKeycodeMap, static_cast<size_t>(key));
			UpModEvent(mKeycodeMap, static_cast<u32>(mods));
			break;
		default:
			break;
	}
}

void InputHandler::FeedMouseMotion(glm::vec2 const& position)
{
  mMousePosition = position;
}

void InputHandler::FeedMouseButtons(int button, int action, int mods)
{
	switch (action)
	{
		case GLFW_PRESS:
			DownEvent(mMouseMap, static_cast<size_t>(button));
			DownModEvent(mMouseMap, static_cast<u32>(mods));
			mMousePositionSwitched[button] = mMousePosition;
			break;
		case GLFW_RELEASE:
			UpEvent(mMouseMap, static_cast<size_t>(button));
			UpModEvent(mMouseMap, static_cast<u32>(mods));
			mMousePositionSwitched[button] = mMousePosition;
			break;
		default:
			break;
	}
}

u32 InputHandler::GetState(std::unordered_map<size_t, IState> &map, size_t loc)
{
	auto sc = map.find(loc);
	if (sc == map.end())
		return RELEASED;
	u32 s = map[static_cast<u32>(loc)].mIsDown ? PRESSED : RELEASED;
	s |= mTick-1 == map[loc].mDownTick ? JUST_PRESSED : 0;
	s |= mTick-1 == map[loc].mUpTick ? JUST_RELEASED : 0;
	return s;
}

u32 InputHandler::GetScancodeState(int scancode)
{
	return GetState(mScancodeMap, static_cast<size_t>(scancode));
}

u32 InputHandler::GetKeycodeState(int  key)
{
	return GetState(mKeycodeMap, static_cast<size_t>(key));
}

u32 InputHandler::GetMouseState(u32 button)
{
	return GetState(mMouseMap, static_cast<size_t>(button));
}

glm::vec2 InputHandler::GetMousePositionAtStateShift(u32 button)
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
