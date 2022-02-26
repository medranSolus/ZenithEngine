#pragma once
#include "GFX/Resource/CBuffer.h"

namespace ZE::Data
{
	// Component containing data needed to render light
	struct LightBuffer
	{
		float Volume;
		GFX::Resource::CBuffer Buffer;
	};
}