#include <iostream>
#include <sstream>
#include <string>

#include <imgui.h>

#include "BuildSettings.h"
#include "GLStateInspectionView.h"

std::vector<std::string> snapshotList;
int snapshotItem = 0;
char **snapshotName;

void GLStateInspection::View::Init()
{
	snapshotName = new char*[256];
}

void GLStateInspection::View::Destroy()
{
	delete [] snapshotName;
}

void GLStateInspection::View::Render()
{
	bool const isWindowExpended = ImGui::Begin("GL state inspection", nullptr, ImGuiWindowFlags_None);
	if (!isWindowExpended) {
		ImGui::End();
		return;
	}

#if defined ENABLE_GL_STATE_INSPECTION && ENABLE_GL_STATE_INSPECTION != 0
	int count = GLStateInspection::SnapshotCount();
	if (count != 0) {
		snapshotList.clear();
		GLStateInspection::GetIdentifiers(snapshotList);
		if (!snapshotList.empty()) {
			for (int i = 0; i < count; i++) {
				snapshotName[i] = new char[1024];
				strncpy(snapshotName[i], snapshotList[i].c_str(), 1023);
			}

			std::stringstream snapshotOs;
			int len = 0;

			ImGui::ListBox("Loc", &snapshotItem, (const char **) snapshotName, count);
			GLStateInspection::ToString(snapshotOs, snapshotItem);
			ImGui::TextWrapped("%s", snapshotOs.str().c_str());

			for (int i = 0; i < count; i++)
				delete [] snapshotName[i];
		}
	}
#else
	ImGui::Text("GL state inspection disabled");
	ImGui::Text("Enable with ENABLE_GL_STATE_INSPECTION in BuildSettings.h");
#endif
	ImGui::End();
}
