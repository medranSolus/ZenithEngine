#include "Data/SceneManager.h"
#include "Data/MaterialPBR.h"
#include "Data/Tags.h"
#include "Data/Transform.h"
#include "GFX/Vertex.h"

namespace ZE::Data
{
#if _ZE_EXTERNAL_MODEL_LOADING
	void ParseNode(const aiNode& node, EID currentEntity, const Data::Transform& topTransform, const std::vector<std::pair<MeshID, MaterialID>>& meshes) noexcept
	{
		Storage& registry = Settings::Data;
		if (!registry.try_get<std::string>(currentEntity))
			registry.emplace<std::string>(currentEntity, node.mName.length != 0 ? node.mName.C_Str() : "node_" + std::to_string(static_cast<U64>(currentEntity)));

		// Load transforms for node
		Vector translation, rotation, scaling;
		if (!Math::XMMatrixDecompose(&scaling, &rotation, &translation,
			Math::XMMatrixTranspose(Math::XMLoadFloat4x4(reinterpret_cast<const Float4x4*>(&node.mTransformation)))))
		{
			translation = { 0.0f, 0.0f, 0.0f, 0.0f };
			rotation = { 0.0f, 0.0f, 0.0f, 1.0f };
			scaling = { 1.0f, 1.0f, 1.0f, 0.0f };
		}

		// Store node transforms without influence of top-level transform
		auto& local = Settings::Data.emplace<Data::Transform>(currentEntity);
		Math::XMStoreFloat4(&local.Rotation, rotation);
		Math::XMStoreFloat3(&local.Position, translation);
		Math::XMStoreFloat3(&local.Scale, scaling);

		// Apply top-level transform and local one as final render transform
		auto& global = Settings::Data.emplace<Data::TransformGlobal>(currentEntity, topTransform);
		Math::XMStoreFloat4(&global.Rotation, Math::XMQuaternionNormalize(Math::XMQuaternionMultiply(Math::XMLoadFloat4(&global.Rotation), rotation)));
		Math::XMStoreFloat3(&global.Position, Math::XMVectorAdd(Math::XMLoadFloat3(&global.Position), translation));
		Math::XMStoreFloat3(&global.Scale, Math::XMVectorMultiply(Math::XMLoadFloat3(&global.Scale), scaling));

		if (Settings::ComputeMotionVectors())
			Settings::Data.emplace<Data::TransformPrevious>(currentEntity, topTransform);

		if (!registry.all_of<Children>(currentEntity))
			registry.emplace<Children>(currentEntity);

		if (node.mNumMeshes)
		{
			registry.emplace<RenderLambertian>(currentEntity);
			registry.emplace<ShadowCaster>(currentEntity);
			registry.emplace<MeshID>(currentEntity, meshes.at(node.mMeshes[0]).first);
			registry.emplace<MaterialID>(currentEntity, meshes.at(node.mMeshes[0]).second);

			// Create child entities for every multiple instances of meshes in this node
			std::vector<EID> childrenEntities(node.mNumMeshes - 1);
			Settings::CreateEntities(childrenEntities);
			for (U32 i = 1; i < node.mNumMeshes; ++i)
			{
				EID child = childrenEntities.at(i - 1);
				registry.emplace<ParentID>(child, currentEntity);
				registry.emplace<RenderLambertian>(child);
				registry.emplace<ShadowCaster>(child);
				registry.emplace<std::string>(child, registry.get<std::string>(currentEntity) + "_" + std::to_string(i));

				registry.emplace<Transform>(child, Transform({ 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }));
				registry.emplace<TransformGlobal>(child, global);
				if (Settings::ComputeMotionVectors())
					registry.emplace<TransformPrevious>(child, global);

				registry.emplace<MeshID>(child, meshes.at(node.mMeshes[i]).first);
				registry.emplace<MaterialID>(child, meshes.at(node.mMeshes[i]).second);
				registry.get<Children>(currentEntity).Childs.emplace_back(child);
			}
		}

		if (node.mNumChildren)
		{
			std::vector<EID> childrenEntities(node.mNumChildren);
			Settings::CreateEntities(childrenEntities);

			for (U32 i = 0; i < node.mNumChildren; ++i)
			{
				EID child = childrenEntities.at(i);
				registry.emplace<ParentID>(child, currentEntity);
				registry.get<Children>(currentEntity).Childs.emplace_back(child);

				ParseNode(*node.mChildren[i], child, global, meshes);
			}
		}

		if (registry.get<Children>(currentEntity).Childs.size() == 0)
			registry.remove<Children>(currentEntity);
	}

	Task<bool> LoadExternalModel(GFX::Device& dev, AssetsStreamer& assets, EID root, const Data::Transform& transform, std::string_view filename, ExternalModelOptions options) noexcept
	{
		ZE_VALID_EID(root);

		return Settings::GetThreadPool().Schedule(ThreadPriority::Normal,
			[&]() -> bool
			{
				Assimp::Importer importer;
				importer.SetPropertyFloat(AI_CONFIG_PP_GSN_MAX_SMOOTHING_ANGLE, 80.0f);
				importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS,
					aiComponent_COLORS | aiComponent_CAMERAS | aiComponent_ANIMATIONS | aiComponent_LIGHTS);
				// aiProcess_FindInstances <- takes a while??
				// aiProcess_GenBoundingBoxes ??? No info
				const aiScene* scene = importer.ReadFile(filename.data(),
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
					Logger::Error("Loading model \"" + std::string(filename) + "\": " + error);
					return false;
				}

				std::filesystem::path filePath(filename);
				bool flipYZ = filePath.extension().string() == ".3ds";
				if (flipYZ)
				{
					// Fix for incorrect format with YZ coords
					float temp = scene->mRootNode->mTransformation.b2;
					scene->mRootNode->mTransformation.b2 = -scene->mRootNode->mTransformation.b3;
					scene->mRootNode->mTransformation.b3 = temp;
					std::swap(scene->mRootNode->mTransformation.c2, scene->mRootNode->mTransformation.c3);
				}
				if (flipYZ || filePath.extension().string() == ".fbx")
				{
					// Fix for model rotated by 90 degrees in X axis
					Float4& rotation = Settings::Data.get<TransformGlobal>(root).Rotation;
					Math::XMStoreFloat4(&rotation,
						Math::XMQuaternionNormalize(Math::XMQuaternionMultiply(Math::XMQuaternionRotationRollPitchYaw(Math::ToRadians(90.0f), 0.0f, 0.0f),
							Math::XMLoadFloat4(&rotation))));
				}

				// Load geometry
				std::vector<Task<MeshID>> meshWaitables;
				meshWaitables.reserve(scene->mNumMeshes);
				for (U32 i = 0; i < scene->mNumMeshes; ++i)
					meshWaitables.emplace_back(assets.ParseMesh(dev, *scene->mMeshes[i]));

				// Load materials
				std::vector<Task<MaterialID>> materialWaitables;
				materialWaitables.reserve(scene->mNumMaterials);
				for (U32 i = 0; i < scene->mNumMaterials; ++i)
					materialWaitables.emplace_back(assets.ParseMaterial(dev, *scene->mMaterials[i], filePath.remove_filename().string(), options));

				// Finish loading geometry
				std::vector<std::pair<MeshID, MaterialID>> meshes;
				meshes.reserve(scene->mNumMeshes);
				for (auto& task : meshWaitables)
					meshes.emplace_back(task.Get(), INVALID_EID);
				meshWaitables.clear();

				// Finish loading materials (after geometry to give more time to process)
				std::vector<MaterialID> materials;
				materials.reserve(scene->mNumMaterials);
				for (auto& task : materialWaitables)
					materials.emplace_back(task.Get());
				materialWaitables.clear();

				// Patch meshes with correct materials
				for (U32 i = 0; i < scene->mNumMeshes; ++i)
					meshes.at(i).second = materials.at(scene->mMeshes[i]->mMaterialIndex);
				materials.clear();

				// Load model structure
				ParseNode(*scene->mRootNode, root, transform, meshes);

				// For root node apply top-level transform as it's set by the user
				Settings::Data.get<Data::Transform>(root) = Settings::Data.get<Data::TransformGlobal>(root);
				return true;
			});
	}
#endif
}