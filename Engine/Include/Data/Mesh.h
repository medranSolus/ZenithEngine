#pragma once

namespace ZE::Data
{
	// Component storing data about single mesh
	struct Mesh
	{
		U64 GeometryIndex;
		U64 MaterialIndex;
	};
}