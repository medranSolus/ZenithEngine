#include "Data/SystemsBank.h"
#include "Data/AssetsStreamer.h"
#include "Data/Camera.h"
#include "Data/Light.h"
#include "Data/MaterialPBR.h"
#include "Data/SceneManager.h"
#include "Data/Tags.h"
#include "Data/Transform.h"
#include "GFX/Resource/Mesh.h"

namespace ZE::Data
{
	void SystemsBank::MergeNode(EID nodeId, EID destId) noexcept
	{
		// Basic entity data
#if !_ZE_GAME_BUILD
		if (!worldData.all_of<std::string>(destId))
			worldData.emplace<std::string>(destId, std::move(loadingData.get<std::string>(nodeId)));
#endif
		worldData.emplace<Transform>(destId, std::move(loadingData.get<Transform>(nodeId)));
		worldData.emplace<TransformGlobal>(destId, std::move(loadingData.get<TransformGlobal>(nodeId)));
		if (Settings::ComputeMotionVectors())
			worldData.emplace<TransformPrevious>(destId, std::move(loadingData.get<TransformPrevious>(nodeId)));

		// Optional tags
		if (loadingData.all_of<RenderLambertian>(nodeId))
			worldData.emplace<RenderLambertian>(destId);
		if (loadingData.all_of<ShadowCaster>(nodeId))
			worldData.emplace<ShadowCaster>(destId);

		// Put referneces to in-flight resources
		if (loadingData.all_of<LoadingMeshID>(nodeId))
			worldData.emplace<LoadingMeshID>(destId, loadingData.get<LoadingMeshID>(nodeId));
		if (loadingData.all_of<LoadingMaterialID>(nodeId))
			worldData.emplace<LoadingMaterialID>(destId, loadingData.get<LoadingMaterialID>(nodeId));

		// Child-parent structures
		if (loadingData.all_of<ChildrenIDs>(nodeId))
		{
			auto& destChildren = worldData.emplace<ChildrenIDs>(destId).Childs;
			for (EID child : loadingData.get<ChildrenIDs>(nodeId).Childs)
			{
				EID destChild = worldData.create();
				worldData.emplace<ParentID>(destChild, destId);
				destChildren.emplace_back(destChild);

				MergeNode(child, destChild);
				loadingData.destroy(child);
			}
		}
	}

	SystemsBank::SystemsBank() noexcept
	{
		// Only assure on global pools that require to be lock-free
		AssureEntityPools<ParentID, ChildrenIDs,
			MeshID, MaterialID, LoadingMeshID, LoadingMaterialID,
			Camera, Transform, TransformGlobal, TransformPrevious,
			RenderLambertian, RenderOutline, RenderWireframe, ShadowCaster,
			LightDirectional, LightSpot, LightPoint>(worldData);

		AssureEntityPools<DirectionalLight, Direction, DirectionalLightBuffer,
			SpotLight, SpotLightBuffer,
			PointLight, PointLightBuffer,
			PBRFlags, MaterialPBR, MaterialBuffersPBR, MaterialTransparent, MaterialBlend,
			AssetsStreamer::PackID,
			Math::BoundingBox, GFX::Resource::Mesh,
			GFX::Resource::CBuffer, GFX::Resource::Texture::Pack>(assetsData);

#if !_ZE_GAME_BUILD
		// These componets won't be used in game-only builds
		AssureEntityPools<std::string>(worldData);
		AssureEntityPools<std::string, IO::CompressionFormat>(assetsData);
#endif
	}

	void SystemsBank::MergeLoadedData() noexcept
	{
		// TODO: maybe some way to remove data that is no longer needed?
		LockGuardRW lock(loadingMutex);

		// Merge all assets
		for (EID assetId : loadingData.view<AssetLoaded>())
		{
			EID destId = assetsData.create();

			// Data shared between assets
			assetsData.emplace<AssetsStreamer::PackID>(destId, loadingData.get<AssetsStreamer::PackID>(assetId));
#if !_ZE_GAME_BUILD
			assetsData.emplace<std::string>(destId, std::move(loadingData.get<std::string>(assetId)));
#endif
			// Resource specific data
			if (loadingData.all_of<GFX::Resource::Mesh>(assetId))
			{
				assetsData.emplace<Math::BoundingBox>(destId, std::move(loadingData.get<Math::BoundingBox>(assetId)));
				assetsData.emplace<GFX::Resource::Mesh>(destId, std::move(loadingData.get<GFX::Resource::Mesh>(assetId)));

				loadingData.emplace<MeshID>(assetId, destId);
			}
			else // Only mesh and materials are supported currently
			{
				assetsData.emplace<MaterialPBR>(destId, std::move(loadingData.get<MaterialPBR>(assetId)));
				assetsData.emplace<PBRFlags>(destId, std::move(loadingData.get<PBRFlags>(assetId)));
				assetsData.emplace<MaterialBuffersPBR>(destId, std::move(loadingData.get<MaterialBuffersPBR>(assetId)));

				if (loadingData.all_of<MaterialTransparent>(assetId))
					assetsData.emplace<MaterialTransparent>(destId);
				if (loadingData.all_of<MaterialBlend>(assetId))
					assetsData.emplace<MaterialBlend>(destId);

				loadingData.emplace<MaterialID>(assetId, destId);
			}
			loadingData.remove<AssetLoaded>(assetId);
		}

		// Populate with object nodes
		for (EID objectId : loadingData.view<WorldObjectLoaded>())
		{
			WorldSourceID destId = loadingData.get<WorldSourceID>(objectId);

			MergeNode(objectId, destId.ID);
			loadingData.destroy(objectId);
		}

		// Patch any existing connections to in-flight resources if they finished
		for (EID objectId : worldData.view<LoadingMeshID>())
		{
			EID meshId = worldData.get<LoadingMeshID>(objectId).ID;

			if (loadingData.all_of<MeshID>(meshId))
			{
				worldData.emplace<MeshID>(objectId, loadingData.get<MeshID>(meshId));
				worldData.remove<LoadingMeshID>(objectId);

				// If no more nodes reference this mesh, remove
				U32& refCount = loadingData.get<AssetsStreamer::RefCount>(meshId).Count;
				ZE_ASSERT(refCount > 0, "When using resource, it's ref count should be at least 1!");
				if (--refCount == 0)
					loadingData.destroy(meshId);
			}
		}
		for (EID objectId : worldData.view<LoadingMaterialID>())
		{
			EID matId = worldData.get<LoadingMaterialID>(objectId).ID;

			if (loadingData.all_of<MaterialID>(matId))
			{
				worldData.emplace<MaterialID>(objectId, loadingData.get<MaterialID>(matId));
				worldData.remove<LoadingMaterialID>(objectId);

				// If no more nodes reference this material, remove
				U32& refCount = loadingData.get<AssetsStreamer::RefCount>(matId).Count;
				ZE_ASSERT(refCount > 0, "When using resource, it's ref count should be at least 1!");
				if (--refCount == 0)
					loadingData.destroy(matId);
			}
		}
	}
}