#pragma once

namespace ZE::GFX::Pipeline
{
	// Describes single mesh to be fed into the pipeline
	struct MeshInfo
	{
		U64 GeometryIndex;
		U64 MaterialIndex;
		U64 TransformIndex;
	};
}