#pragma once
#include "Types.h"
#include "SlotsGen.h"

namespace ZE::GFX
{
	// Bind slots used by shaders
	enum class ShaderSlot : U32
	{
		Normal,
		Specular,
		ZE_GEN_SLOTS(ZE_MAX_SLOTS, Slot_)
	};
}