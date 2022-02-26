#pragma once

namespace ZE::GFX::Pipeline::Info
{
	// Describes single light source to be fed into the pipeline
	struct Light
	{
		U64 TransformIndex;
	};
}