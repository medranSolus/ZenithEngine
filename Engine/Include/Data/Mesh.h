#pragma once

namespace ZE::Data
{
	typedef U8 MeshFlags;
	// Options to display mesh with
	enum MeshFlag : MeshFlags
	{
		Shadow = 1,
		Outline = 2,
		Wireframe = 4
	};

	// Component storing data about single mesh
	struct Mesh
	{
		U64 GeometryIndex;
		U64 MaterialIndex;
		MeshFlags Flags;
	};
}