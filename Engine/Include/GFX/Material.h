#pragma once
#include "Device.h"

namespace ZE::GFX
{
	// Class containing material data of graphics objects
	class Material final
	{
	public:
		Material(Device& dev, U64 id) {}
		ZE_CLASS_MOVE(Material);
		~Material() = default;
	};
}