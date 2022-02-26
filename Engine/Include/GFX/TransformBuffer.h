#pragma once

namespace ZE::GFX
{
	// Single transform data for model
	struct ModelTransform
	{
		Matrix Model;
		Matrix ModelViewProjection;
	};

	// Transform buffer for rendering mutliple meshes at once
	struct ModelTransformBuffer
	{
		// After changin content of the transform buffer, set size of transform array in
		// Engine/Shader/VS/CB/ModelTransform.hlsli
		static constexpr U32 TRANSFORM_COUNT = 512;

		ModelTransform Transforms[TRANSFORM_COUNT];
	};

	// Typical simple transform buffer needed by most shaders
	struct TransformBuffer
	{
		// After changin content of the transform buffer, set size of transform array in
		// Engine/Shader/VS/CB/Transform.hlsli
		static constexpr U32 TRANSFORM_COUNT = 1024;

		Matrix Transforms[TRANSFORM_COUNT];
	};
}