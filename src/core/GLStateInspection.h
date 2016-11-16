/*
 * GL state inspection
 */

#pragma once
#include <string>
#include <vector>
#include "BuildSettings.h"

namespace GLStateInspection {

void Init();
void Destroy();
void CaptureSnapshot(std::string uniqueIdentifier);
bool ToString(std::ostream &os, std::string uniqueIdentifier);
bool ToString(std::ostream &os, int index);
int SnapshotCount();
void GetIdentifiers(std::vector<std::string> &list);

};

#if defined ENABLE_GL_STATE_INSPECTION && ENABLE_GL_STATE_INSPECTION != 0
	#define INSPECT_GL_STATE(a)		GLStateInspection::CaptureSnapshot(a)
#else
	#define INSPECT_GL_STATE(a)
#endif
