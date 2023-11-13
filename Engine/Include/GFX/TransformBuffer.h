#pragma once

namespace ZE::GFX
{
	// Single transform data for model
	struct ModelTransformBuffer
	{
		Matrix Model;
		Matrix ModelViewProjection;
	};

	// Single transform data for model with previous MVP matrix for motion computation
	struct ModelTransformBufferMotion
	{
		Matrix Model;
		Matrix ModelViewProjection;
		Matrix PrevModelViewProjection;
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