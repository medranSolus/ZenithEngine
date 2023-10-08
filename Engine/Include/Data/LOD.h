#pragma once
#include "Entity.h"
#include "GFX/Resource/Mesh.h"

namespace ZE::Data
{
	struct Geometry
	{
		GFX::Resource::Mesh MeshData;
		//BLAS BlasData;
	};

	struct GeometryLOD
	{
		// Top level LOD number
		static constexpr U8 BASE_LOD = 0;

		std::unique_ptr<Geometry[]> GeometryLevels;
		U8 LevelCount;

		Geometry& GetGeometry(U8 lod) noexcept { return GeometryLevels[Math::Clamp(lod, BASE_LOD, LevelCount)]; }
	};

	struct Model
	{
		EID ParentID = INVALID_EID;
		EID GeometryLodID;
		std::vector<EID> Children;
	};
}

/*

[Entity] + ... + [Model]
				  |     \
	   [Ent],[Ent],...   [GeometryLOD]

*/