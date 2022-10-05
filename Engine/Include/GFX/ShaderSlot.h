#pragma once
#include "SlotsGen.h"
#include "Types.h"

namespace ZE::GFX
{
	// Bind slots used by shaders
	enum ShaderSlot : U32
	{
		Normal,
		Specular,
		ZE_GEN_SLOTS(Slot_)
	};
}