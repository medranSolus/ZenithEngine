#pragma once
#include "Data/Scene.h"
#include "MeshInfo.h"

namespace ZE::GFX::Pipeline
{
	struct WorldInfo
	{
		Ptr<const Data::Scene> ActiveScene;
		Data::EID CurrnetCamera = Data::Entity::INVALID_ID;

		TableInfo<U64> MeshesInfo;
		Ptr<MeshInfo> Meshes;

		TableInfo<U64> OutlinesInfo;
		Ptr<MeshInfo> Outlines;

		TableInfo<U64> WireframesInfo;
		Ptr<MeshInfo> Wireframes;
	};
}