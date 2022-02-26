#pragma once
#include "GFX/Resource/CBuffer.h"
#include "Data/Scene.h"
#include "Light.h"
#include "Mesh.h"

namespace ZE::GFX::Pipeline::Info
{
	// Constantly changing world data for shaders
	struct DynamicWorldData
	{
		Matrix ViewProjection;
		Matrix ViewProjectionInverse;
		Float3 CameraPos;
	};

	// Information about current scene
	struct World
	{
		Ptr<const Data::Scene> ActiveScene;
		Data::EID CurrnetCamera = Data::Entity::INVALID_ID;

		DynamicWorldData DynamicData;
		Resource::CBuffer DynamicDataBuffer;

		TableInfo<U64> MeshInfo;
		Ptr<Mesh> Meshes;

		TableInfo<U64> ShadowCasterInfo;
		Ptr<Mesh> ShadowCasters;

		TableInfo<U64> OutlineInfo;
		Ptr<Mesh> Outlines;

		TableInfo<U64> WireframeInfo;
		Ptr<Mesh> Wireframes;

		TableInfo<U64> PointLightInfo;
		Ptr<Light> PointLights;
	};
}