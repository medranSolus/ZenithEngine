#pragma once

namespace ZE::Data
{
	typedef U8 MeshFlags;
	// Options to display mesh with
	enum MeshFlag : MeshFlags
	{
		Outline,
		Wireframe
	};

	// Component storing data about single mesh
	struct Mesh
	{
		U64 GeometryIndex;
		U64 MaterialIndex;
		MeshFlags Flags;
	};
}