#include "Data/SceneManager.h"
#include "Data/Tags.h"
#include "Data/Transform.h"
#if _ZE_EXTERNAL_MODEL_LOADING
ZE_WARNING_PUSH
#	include "assimp/Importer.hpp"
#	include "assimp/postprocess.h"
ZE_WARNING_POP
#endif

namespace ZE::Data
{
#if _ZE_EXTERNAL_MODEL_LOADING
	void ParseNode(const aiNode& node, EID currentEntity, const Data::Transform& topTransform, std::vector<std::vector<EID>>& meshReferences) noexcept
	{
		// Load transforms for node
		Vector translation = {}, rotation = {}, scaling = {};
		if (!Math::XMMatrixDecompose(&scaling, &rotation, &translation,
			Math::XMMatrixTranspose(Math::XMLoadFloat4x4(reinterpret_cast<const Float4x4*>(&node.mTransformation)))))
		{
			translation = { 0.0f, 0.0f, 0.0f, 0.0f };
			rotation = { 0.0f, 0.0f, 0.0f, 1.0f };
			scaling = { 1.0f, 1.0f, 1.0f, 0.0f };
		}

		// Store node transforms without influence of top-level transform
		Transform local = {};
		Math::XMStoreFloat4(&local.Rotation, rotation);
		Math::XMStoreFloat3(&local.Position, translation);
		Math::XMStoreFloat3(&local.Scale, scaling);

		// Apply top-level transform and local one as final render transform
		TransformGlobal global = static_cast<TransformGlobal>(topTransform);
		Math::XMStoreFloat4(&global.Rotation, Math::XMQuaternionNormalize(Math::XMQuaternionMultiply(Math::XMLoadFloat4(&global.Rotation), rotation)));
		Math::XMStoreFloat3(&global.Position, Math::XMVectorAdd(Math::XMLoadFloat3(&global.Position), translation));
		Math::XMStoreFloat3(&global.Scale, Math::XMVectorMultiply(Math::XMLoadFloat3(&global.Scale), scaling));

		std::vector<EID> childrenEntities;
		{
			LockGuardRW lock(Settings::DataBank.GetLoadingLock());

			auto& dataStorage = Settings::DataBank.GetLoadingData();
#if !_ZE_GAME_BUILD
			dataStorage.emplace<std::string>(currentEntity, node.mName.length != 0 ? node.mName.C_Str() : "node");
#endif
			dataStorage.emplace<Transform>(currentEntity, local);
			dataStorage.emplace<TransformGlobal>(currentEntity, global);

			if (Settings::ComputeMotionVectors())
				dataStorage.emplace<TransformPrevious>(currentEntity, global);

			auto& children = dataStorage.emplace<ChildrenIDs>(currentEntity).Childs;
			if (node.mNumChildren)
			{
				for (U32 i = 0; i < node.mNumChildren; ++i)
				{
					EID child = dataStorage.create();
					dataStorage.emplace<ParentID>(child, currentEntity);
					children.emplace_back(child);
				}
				childrenEntities = children; // Copy for thread safety
			}

			if (node.mNumMeshes)
			{
				dataStorage.emplace<RenderLambertian>(currentEntity);
				dataStorage.emplace<ShadowCaster>(currentEntity);
				// Mark current entity for mesh reference
				meshReferences.at(node.mMeshes[0]).emplace_back(currentEntity);

				// Create child entities for every multiple instances of meshes in this node
				for (U32 i = 1; i < node.mNumMeshes; ++i)
				{
					EID child = dataStorage.create();
#if !_ZE_GAME_BUILD
					dataStorage.emplace<std::string>(child, dataStorage.get<std::string>(currentEntity) + "_" + std::to_string(i));
#endif
					dataStorage.emplace<Transform>(child, Transform({ 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }));
					dataStorage.emplace<TransformGlobal>(child, global);
					if (Settings::ComputeMotionVectors())
						dataStorage.emplace<TransformPrevious>(child, global);

					dataStorage.emplace<RenderLambertian>(child);
					dataStorage.emplace<ShadowCaster>(child);
					meshReferences.at(node.mMeshes[i]).emplace_back(child);

					dataStorage.emplace<ParentID>(child, currentEntity);
					children.emplace_back(child);
				}
			}

			if (children.size() == 0)
				dataStorage.remove<ChildrenIDs>(currentEntity);
		}

		// Release lock and go over next nodes
		for (U32 i = 0; EID child : childrenEntities)
			ParseNode(*node.mChildren[i++], child, global, meshReferences);
	}

	Task<Status> LoadExternalModel(GFX::Device& dev, AssetsStreamer& assets, EID root, const Data::Transform& transform, std::string_view filename, ExternalModelOptions options) noexcept
	{
		ZE_VALID_EID(root);

		return Settings::GetThreadPool().Schedule(ThreadPriority::Normal,
			[&dev, &assets, file = std::string(filename), root = root, transform = transform, options = options]() noexcept -> Status
			{
				Assimp::Importer importer;
				importer.SetPropertyFloat(AI_CONFIG_PP_GSN_MAX_SMOOTHING_ANGLE, 80.0f);
				importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS,
					aiComponent_COLORS | aiComponent_CAMERAS | aiComponent_ANIMATIONS | aiComponent_LIGHTS);
				// aiProcess_FindInstances <- takes a while??
				// aiProcess_GenBoundingBoxes ??? No info
				const aiScene* scene = importer.ReadFile(file.c_str(),
					aiProcess_MakeLeftHanded |
					aiProcess_FlipWindingOrder |
					(options & ExternalModelOption::FlipUV ? aiProcess_FlipUVs : 0) |
					aiProcess_Triangulate |
					aiProcess_JoinIdenticalVertices |
					aiProcess_RemoveComponent |
					aiProcess_GenSmoothNormals |
					aiProcess_CalcTangentSpace |
					aiProcess_GenUVCoords |
					aiProcess_TransformUVCoords |
					aiProcess_SortByPType |
					aiProcess_ImproveCacheLocality |
					aiProcess_FindInvalidData |
					aiProcess_RemoveRedundantMaterials |
					aiProcess_ValidateDataStructure |
					//aiProcess_OptimizeGraph | // Use when disabling all scene edition, for almost 2x performance hit
					aiProcess_OptimizeMeshes);

				const char* error = importer.GetErrorString();
				if (!scene || std::strlen(error))
				{
					Logger::Error("Loading model \"" + file + "\": " + error);
					return std::make_error_code(std::errc::io_error);
				}

				// Load geometry
				std::vector<Task<Expected<EID>>> meshWaitables;
				meshWaitables.reserve(scene->mNumMeshes);
				for (U32 i = 0; i < scene->mNumMeshes; ++i)
					meshWaitables.emplace_back(assets.ParseMesh(dev, *scene->mMeshes[i]));

				// Load materials
				std::string pathDir = std::filesystem::path(file).remove_filename().string();
				std::vector<Task<Expected<EID>>> materialWaitables;
				materialWaitables.reserve(scene->mNumMaterials);
				for (U32 i = 0; i < scene->mNumMaterials; ++i)
					materialWaitables.emplace_back(assets.ParseMaterial(dev, *scene->mMaterials[i], pathDir, options));

				// Load model structure to shadow registry
				auto& dataStorage = Settings::DataBank.GetLoadingData();
				std::vector<std::vector<EID>> meshRefs;
				meshRefs.resize(scene->mNumMeshes);
				EID loadingRoot = INVALID_EID;
				{
					LockGuardRW lock(Settings::DataBank.GetLoadingLock());
					loadingRoot = dataStorage.create();
					dataStorage.emplace<SystemsBank::WorldSourceID>(loadingRoot, root);
				}
				ParseNode(*scene->mRootNode, loadingRoot, transform, meshRefs);

				// Finish loading geometry
				std::vector<LoadingMeshID> meshes;
				meshes.reserve(scene->mNumMeshes);
				for (auto& task : meshWaitables)
				{
					Expected<EID> expId = {};
					ZE_EXPECT_RET_FAILED_CODE(expId, task.Get());
					if (expId)
						meshes.emplace_back(*expId);
					else
					{
						ZE_CODE_RET_FAILED(expId.error());
					}
				}
				meshWaitables.clear();

				// Finish loading materials (after geometry to give more time to process)
				std::vector<LoadingMaterialID> materials;
				materials.reserve(scene->mNumMaterials);
				for (auto& task : materialWaitables)
				{
					Expected<EID> expId = {};
					ZE_EXPECT_RET_FAILED_CODE(expId, task.Get());
					if (expId)
						materials.emplace_back(*expId);
					else
					{
						ZE_CODE_RET_FAILED(expId.error());
					}
				}
				materialWaitables.clear();

				LockGuardRW lock(Settings::DataBank.GetLoadingLock());
				// Resolve meshes and materials for each node
				for (U32 i = 0; const auto& meshInstance : meshRefs)
				{
					ZE_ASSERT(meshInstance.size(), "Ill-formed model, each mesh should be referenced!");

					// Patch mesh with correct material
					LoadingMeshID meshId = meshes.at(i);
					LoadingMaterialID matId = materials.at(scene->mMeshes[i++]->mMaterialIndex);

					// Set correct number of references
					dataStorage.get<AssetsStreamer::RefCount>(meshId.ID).Count = Utils::SafeCast<U32>(meshInstance.size());
					dataStorage.get<AssetsStreamer::RefCount>(matId.ID).Count += Utils::SafeCast<U32>(meshInstance.size());

					for (EID node : meshInstance)
					{
						// Emplace with correct references
						dataStorage.emplace<LoadingMeshID>(node, meshId);
						dataStorage.emplace<LoadingMaterialID>(node, matId);
					}
				}

				// For root node apply top-level transform as it's set by the user
				dataStorage.get<Data::Transform>(loadingRoot) = dataStorage.get<Data::TransformGlobal>(loadingRoot);

				// Mark that this object tree is ready for merging
				dataStorage.emplace<SystemsBank::WorldObjectLoaded>(loadingRoot);
				return {};
			});
	}
#endif
}