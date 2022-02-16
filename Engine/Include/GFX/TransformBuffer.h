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
	struct TransformBuffer
	{
		// After changin content of the transform buffer, set size of transform array in
		// Engine/Shader/VS/CB/Transform.hlsli
		static constexpr U32 TRANSFORM_COUNT = 511;

		Float3 CameraPos;
		float _Padding;
		ModelTransform Transforms[TRANSFORM_COUNT];
	};
}