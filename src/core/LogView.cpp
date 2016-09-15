#include <imgui.h>

#include "Log.h"
#include "LogView.h"

#ifdef _WIN32
#pragma warning (disable : 4996) // This function or variable may be unsafe
#endif

char Log::View::mOutput[BUFFER_ROWS * BUFFER_WIDTH * 2];
char Log::View::mBuffer[BUFFER_ROWS][BUFFER_WIDTH];
int Log::View::mLen[BUFFER_ROWS];
Log::Type Log::View::mType[BUFFER_ROWS];
int Log::View::mBufferPtr = 0;
static ImVec4 logViewTypeColor[Log::N_TYPES];

void Log::View::Init()
{
	memset(mBuffer, 0, BUFFER_ROWS * BUFFER_WIDTH);
	for (int i = 0; i < BUFFER_ROWS; i++)
		mType[i] = Log::TYPE_TRIVIA;
	Log::SetCustomOutputTargetFunc(Feed);
	for (int i = 0; i < Log::N_TYPES; i++)
		logViewTypeColor[i] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // default color
	logViewTypeColor[Log::TYPE_WARNING	] = ImVec4(0.7f, 0.4f, 0.0f, 1.0f);
	logViewTypeColor[Log::TYPE_ERROR	] = ImVec4(0.7f, 0.0f, 0.0f, 1.0f);
	logViewTypeColor[Log::TYPE_ASSERT	] = ImVec4(0.7f, 0.0f, 0.0f, 1.0f);
	logViewTypeColor[Log::TYPE_PARAM	] = ImVec4(0.7f, 0.0f, 0.0f, 1.0f);
	logViewTypeColor[Log::TYPE_TRIVIA	] = ImVec4(0.8f, 0.8f, 0.8f, 1.0f);
}

void Log::View::Destroy()
{

}

void Log::View::Render()
{
	bool opened = ImGui::Begin("Log", nullptr, ImVec2(600, 400), -1.0f, 0);
	if (opened) {
		for (int i = 0; i < BUFFER_ROWS; i++) {
			int pos = (BUFFER_ROWS + (mBufferPtr + i)) % BUFFER_ROWS;
			if (mLen[pos] == 0)
				continue;
			ImGui::PushStyleColor(ImGuiCol_Text, logViewTypeColor[mType[pos]]);
			ImGui::TextWrapped(mBuffer[pos]);
			ImGui::PopStyleColor();
		}
	}
	ImGui::End();
}

void Log::View::Feed(Log::Type type, const char *msg)
{
	strncpy(mBuffer[mBufferPtr], msg, BUFFER_WIDTH - 1);
	mLen[mBufferPtr] = (int) strlen(msg);
	mType[mBufferPtr] = type;
	mBufferPtr = (mBufferPtr + 1) % BUFFER_ROWS;
}
