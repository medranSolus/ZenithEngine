#pragma once

namespace ZE::GFX::Pipeline::Info
{
	// Describes single mesh to be fed into the pipeline
	struct Mesh
	{
		U64 GeometryIndex;
		U64 MaterialIndex;
		U64 TransformIndex;
	};
}