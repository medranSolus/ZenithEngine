#pragma once
#include "GFX/Resource/InputParam.h"

namespace ZE::GFX
{
	// Single vertex layout used by the engine
	struct Vertex
	{
		Float3 Position;
		Float3 Normal;
		Float2 UV;
		Float3 Tangent;

		// Get input layout of the vertex
		static std::vector<Resource::InputParam> GetLayout() noexcept
		{
			return
			{
				Resource::InputParam::Pos3D, Resource::InputParam::Normal,
				Resource::InputParam::TexCoord, Resource::InputParam::Tangent
			};
		}
	};
}