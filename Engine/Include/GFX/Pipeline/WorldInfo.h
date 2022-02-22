#pragma once
#include "GFX/Resource/CBuffer.h"
#include "Data/Scene.h"
#include "MeshInfo.h"

namespace ZE::GFX::Pipeline
{
	// Constantly changing world data for shaders
	struct DynamicWorldData
	{
		Matrix ViewProjection;
		Matrix ViewProjectionInverse;
		Float3 CameraPos;
	};

	// Information about current scene
	struct WorldInfo
	{
		Ptr<const Data::Scene> ActiveScene;
		Data::EID CurrnetCamera = Data::Entity::INVALID_ID;

		DynamicWorldData DynamicData;
		Resource::CBuffer DynamicDataBuffer;

		TableInfo<U64> MeshesInfo;
		Ptr<MeshInfo> Meshes;

		TableInfo<U64> OutlinesInfo;
		Ptr<MeshInfo> Outlines;

		TableInfo<U64> WireframesInfo;
		Ptr<MeshInfo> Wireframes;
	};
}