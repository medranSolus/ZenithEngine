#pragma once
ZE_WARNING_PUSH
#define IMGUI_API
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
ZE_WARNING_POP

namespace ZE::GUI
{
	// Clamp value to selected range only if it changed
	template<typename T>
	constexpr bool InputClamp(T min, T max, T& val, bool changed) noexcept
	{
		if (changed)
		{
			if (min > val)
				val = min;
			else if (max < val)
				val = max;
		}
		return changed;
	}
}