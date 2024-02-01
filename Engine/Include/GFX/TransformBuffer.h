#pragma once

namespace ZE::GFX
{
#pragma pack(push, 1)
	// Single transform data for model
	struct ModelTransformBuffer
	{
		Float4x4 ModelTps;
		Float4x4 ModelViewProjectionTps;
	};

	// Single transform data for model with previous MVP matrix for motion computation
	struct ModelTransformBufferMotion
	{
		Float4x4 ModelTps;
		Float4x4 ModelViewProjectionTps;
		Float4x4 PrevModelViewProjectionTps;
	};

	// Typical simple transform buffer needed by most shaders
	struct TransformBuffer
	{
		Float4x4 TransformTps;
	};

	// Single view data for cube map
	struct CubeViewBuffer
	{
		Float4x4 ViewProjectionTps[6];
	};
#pragma pack(pop)
}