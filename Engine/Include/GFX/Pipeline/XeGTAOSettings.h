#pragma once
ZE_WARNING_PUSH
#include "XeGTAO.h"
ZE_WARNING_POP

namespace ZE::GFX::Pipeline
{
	// Data needed by XeGTAO
	struct XeGTAOSettings
	{
		::XeGTAO::GTAOSettings Settings;
		float SliceCount;
		float StepsPerSlice;
	};
}