#pragma once

namespace ZE::GFX
{
	// Single transform data for model
	struct ModelTransformBuffer
	{
		Matrix Model;
		Matrix ModelViewProjection;
	};

	// Typical simple transform buffer needed by most shaders
	struct TransformBuffer
	{
		Matrix Transform;
	};

	// Single view data for cube map
	struct CubeViewBuffer
	{
		Matrix ViewProjection[6];
	};
}