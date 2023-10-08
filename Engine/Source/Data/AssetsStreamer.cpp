#include "Data/AssetsStreamer.h"
#include "Data/MaterialPBR.h"
#include "Data/Tags.h"
#include "Data/Transform.h"
#include "GFX/Vertex.h"

namespace ZE::Data
{
#if _ZE_EXTERNAL_MODEL_LOADING
	template<typename Index>
	void AssetsStreamer::ParseIndices(std::unique_ptr<Index[]>& indices, const aiMesh& mesh) noexcept
	{
		for (U32 i = 0, index = 0; i < mesh.mNumFaces; ++i)
		{
			const aiFace& face = mesh.mFaces[i];
			ZE_ASSERT(face.mNumIndices == 3, "Mesh face is not a triangle!");

			indices[index++] = Utils::SafeCast<Index>(face.mIndices[0]);
			indices[index++] = Utils::SafeCast<Index>(face.mIndices[1]);
			indices[index++] = Utils::SafeCast<Index>(face.mIndices[2]);
		}
	}

	void AssetsStreamer::ParseNode(const aiNode& node, EID currentEntity, const std::vector<std::pair<EID, U32>>& meshes,
		const std::vector<EID>& materials, Storage& registry, bool loadNames) noexcept
	{
		if (loadNames && !registry.all_of<std::string>(currentEntity))
			registry.emplace<std::string>(currentEntity, node.mName.length != 0 ? node.mName.C_Str() : "node_" + std::to_string(static_cast<U64>(currentEntity)));

		TransformGlobal& transform = registry.get<TransformGlobal>(currentEntity);
		const Vector globalRotation = Math::XMLoadFloat4(&transform.Rotation);
		const Vector globalPosition = Math::XMLoadFloat3(&transform.Position);
		const Vector globalScale = Math::XMLoadFloat3(&transform.Scale);

		if (!registry.all_of<Children>(currentEntity))
			registry.emplace<Children>(currentEntity);

		if (node.mNumMeshes)
		{
			registry.emplace<RenderLambertian>(currentEntity);
			registry.emplace<ShadowCaster>(currentEntity);
			registry.emplace<MeshID>(currentEntity, meshes.at(node.mMeshes[0]).first);
			registry.emplace<MaterialID>(currentEntity, materials.at(meshes.at(node.mMeshes[0]).second));
			// Create child entities for every multiple instances of meshes in this node
			for (U32 i = 1; i < node.mNumMeshes; ++i)
			{
				EID child = registry.create();
				registry.emplace<ParentID>(child, currentEntity);
				registry.emplace<RenderLambertian>(child);
				registry.emplace<ShadowCaster>(child);
				if (loadNames)
					registry.emplace<std::string>(child, registry.get<std::string>(currentEntity) + "_" + std::to_string(i));

				registry.emplace<Transform>(child, Transform({ 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }));
				registry.emplace<TransformGlobal>(child, transform);
				if (Settings::ComputeMotionVectors())
					registry.emplace<TransformPrevious>(child, transform);
				registry.emplace<MeshID>(child, meshes.at(node.mMeshes[i]).first);
				registry.emplace<MaterialID>(child, materials.at(meshes.at(node.mMeshes[i]).second));
				registry.get<Children>(currentEntity).Childs.emplace_back(child);
			}
		}

		for (U32 i = 0; i < node.mNumChildren; ++i)
		{
			EID child = registry.create();
			registry.emplace<ParentID>(child, currentEntity);
			registry.get<Children>(currentEntity).Childs.emplace_back(child);

			Vector translation, rotation, scaling;
			if (!Math::XMMatrixDecompose(&scaling, &rotation, &translation,
				Math::XMMatrixTranspose(Math::XMLoadFloat4x4(reinterpret_cast<const Float4x4*>(&node.mTransformation)))))
			{
				translation = { 0.0f, 0.0f, 0.0f, 0.0f };
				rotation = { 0.0f, 0.0f, 0.0f, 1.0f };
				scaling = { 1.0f, 1.0f, 1.0f, 0.0f };
			}

			Transform& local = registry.emplace<Transform>(child);
			Math::XMStoreFloat4(&local.Rotation, rotation);
			Math::XMStoreFloat3(&local.Position, translation);
			Math::XMStoreFloat3(&local.Scale, scaling);

			Transform& global = registry.emplace<TransformGlobal>(child);
			Math::XMStoreFloat4(&global.Rotation, Math::XMQuaternionNormalize(Math::XMQuaternionMultiply(globalRotation, rotation)));
			Math::XMStoreFloat3(&global.Position, Math::XMVectorAdd(globalPosition, translation));
			Math::XMStoreFloat3(&global.Scale, Math::XMVectorMultiply(globalScale, scaling));
			if (Settings::ComputeMotionVectors())
				registry.emplace<TransformPrevious>(child, global);

			ParseNode(*node.mChildren[i], child, meshes, materials, registry, loadNames);
		}

		if (registry.get<Children>(currentEntity).Childs.size() == 0)
			registry.remove<Children>(currentEntity);
	}

	EID AssetsStreamer::ParseMesh(GFX::Device& dev, const aiMesh& mesh, bool loadNames)
	{
		// Gather index data and parse it into continuous array
		std::unique_ptr<U32[]> indicesU32;
		std::unique_ptr<U16[]> indicesU16;
		std::unique_ptr<U8[]> indicesU8;

		GFX::Resource::MeshData meshData = {};
		meshData.VertexCount = mesh.mNumVertices;
		meshData.IndexCount = mesh.mNumFaces * 3;
		meshData.VertexSize = sizeof(GFX::Vertex);

		if (meshData.IndexCount >= UINT16_MAX)
		{
			indicesU32 = std::make_unique<U32[]>(meshData.IndexCount);
			meshData.IndexSize = sizeof(U32);
			meshData.Indices = indicesU32.get();
			ParseIndices(indicesU32, mesh);
		}
		else if (!Settings::IsEnabledU8IndexBuffers() || meshData.IndexCount >= UINT8_MAX)
		{
			indicesU16 = std::make_unique<U16[]>(meshData.IndexCount);
			meshData.IndexSize = sizeof(U16);
			meshData.Indices = indicesU16.get();
			ParseIndices(indicesU16, mesh);
		}
		else
		{
			indicesU8 = std::make_unique<U8[]>(meshData.IndexCount);
			meshData.IndexSize = sizeof(U8);
			meshData.Indices = indicesU8.get();
			ParseIndices(indicesU8, mesh);
		}

		// Parse vertex data into structured format
		Vector min = { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX };
		Vector max = { -FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX };

		std::unique_ptr<GFX::Vertex[]> vertices = std::make_unique<GFX::Vertex[]>(mesh.mNumVertices);
		meshData.Vertices = vertices.get();
		for (U32 i = 0; i < mesh.mNumVertices; ++i)
		{
			GFX::Vertex& vertex = vertices[i];

			vertex.Position = *reinterpret_cast<Float3*>(mesh.mVertices + i);
			ZE_ASSERT(vertex.Position.x == mesh.mVertices[i].x
				&& vertex.Position.y == mesh.mVertices[i].y
				&& vertex.Position.z == mesh.mVertices[i].z,
				"Position data is incorrect, type of aiMesh::mVertices changed!");
			// Find min/max position values to determine bounding box
			const Vector pos = Math::XMLoadFloat3(&vertex.Position);
			min = Math::XMVectorMin(min, pos);
			max = Math::XMVectorMax(max, pos);

			vertex.Normal = *reinterpret_cast<Float3*>(mesh.mNormals + i);
			ZE_ASSERT(vertex.Normal.x == mesh.mNormals[i].x
				&& vertex.Normal.y == mesh.mNormals[i].y
				&& vertex.Normal.z == mesh.mNormals[i].z,
				"Normal data is incorrect, type of aiMesh::mNormals changed!");

			if (mesh.HasTextureCoords(0))
			{
				vertex.UV = *reinterpret_cast<Float2*>(mesh.mTextureCoords[0] + i);
				ZE_ASSERT(vertex.UV.x == mesh.mTextureCoords[0][i].x
					&& vertex.UV.y == mesh.mTextureCoords[0][i].y,
					"UV data is incorrect, type of aiMesh::mTextureCoords changed!");
			}
			if (mesh.HasTangentsAndBitangents())
			{
				vertex.Tangent.x = mesh.mTangents[i].x;
				vertex.Tangent.y = mesh.mTangents[i].y;
				vertex.Tangent.z = mesh.mTangents[i].z;
				//vertex.Tangent.w = -1.0f; // TODO: Check somehow
			}
		}

		// Create main mesh data
		EID meshId = assets.create();
		if (loadNames)
			assets.emplace<std::string>(meshId, mesh.mName.length != 0 ? mesh.mName.C_Str() : "mesh_" + std::to_string(static_cast<U64>(meshId)));
		assets.emplace<Math::BoundingBox>(meshId, Math::GetBoundingBox(max, min));

		// Load parsed mesh data into correct asset
		//assets.emplace<GFX::Resource::Mesh>(meshId).Init(dev, meshData);

		return meshId;
	}

	EID AssetsStreamer::ParseMaterial(GFX::Device& dev, const aiMaterial& material,
		const GFX::Resource::Texture::Schema& texSchema, const std::string& path, bool loadNames)
	{
		EID materialId = assets.create();
		if (loadNames)
			assets.emplace<std::string>(materialId, material.GetName().length != 0 ?
				material.GetName().C_Str() : "material_" + std::to_string(static_cast<U64>(materialId)));

		MaterialPBR& data = assets.emplace<MaterialPBR>(materialId);
		MaterialBuffersPBR& buffers = assets.emplace<MaterialBuffersPBR>(materialId);
		PBRFlags& flags = assets.emplace<PBRFlags>(materialId);

		GFX::Resource::Texture::PackDesc texDesc;
		texDesc.Init(texSchema);

		std::vector<GFX::Surface> surfaces;
		aiString texFile;
		bool notSolid = false;

		// Get diffuse texture
		if (material.GetTexture(aiTextureType_DIFFUSE, 0, &texFile) == aiReturn_SUCCESS)
		{
			surfaces.emplace_back(path + texFile.C_Str());
			notSolid |= surfaces.back().HasAlpha();

			texDesc.AddTexture(texSchema, MaterialPBR::TEX_COLOR_NAME, std::move(surfaces));
			flags |= MaterialPBR::Flag::UseTexture;
		}

		// Get normal map texture
		if (material.GetTexture(aiTextureType_NORMALS, 0, &texFile) == aiReturn_SUCCESS)
		{
			surfaces.emplace_back(path + texFile.C_Str());
			texDesc.AddTexture(texSchema, MaterialPBR::TEX_NORMAL_NAME, std::move(surfaces));
			flags |= MaterialPBR::Flag::UseNormal;
		}

		// Get specular data
		if (material.GetTexture(aiTextureType_SPECULAR, 0, &texFile) == aiReturn_SUCCESS)
		{
			surfaces.emplace_back(path + texFile.C_Str());
			if (surfaces.back().HasAlpha())
				data.Flags |= MaterialPBR::Flag::UseSpecularPowerAlpha;
			texDesc.AddTexture(texSchema, MaterialPBR::TEX_SPECULAR_NAME, std::move(surfaces));
			flags |= MaterialPBR::Flag::UseSpecular;
		}

		// Get parallax map texture
		if (material.GetTexture(aiTextureType_HEIGHT, 0, &texFile) == aiReturn_SUCCESS)
		{
			surfaces.emplace_back(path + texFile.C_Str());
			texDesc.AddTexture(texSchema, MaterialPBR::TEX_HEIGHT_NAME, std::move(surfaces));
			flags |= MaterialPBR::Flag::UseParallax;
			notSolid = true;
		}

		if (material.Get(AI_MATKEY_COLOR_DIFFUSE, reinterpret_cast<aiColor4D&>(data.Color)) != aiReturn_SUCCESS)
			data.Color = { 0.0f, 0.8f, 1.0f };
		else if (data.Color.RGBA.w != 1.0f)
			notSolid = true;

		if (material.Get(AI_MATKEY_COLOR_SPECULAR, reinterpret_cast<aiColor3D&>(data.Specular)) != aiReturn_SUCCESS)
			data.Specular = { 1.0f, 1.0f, 1.0f };

		if (material.Get(AI_MATKEY_SHININESS_STRENGTH, data.SpecularIntensity) != aiReturn_SUCCESS)
			data.SpecularIntensity = 0.9f;

		if (material.Get(AI_MATKEY_SHININESS, data.SpecularPower) != aiReturn_SUCCESS)
			data.SpecularPower = 0.409f;
		else if (data.SpecularPower > 1.0f)
			data.SpecularPower = Utils::SafeCast<float>(log(static_cast<double>(data.SpecularPower)) / log(8192.0));

		if (material.Get(AI_MATKEY_BUMPSCALING, data.ParallaxScale) != aiReturn_SUCCESS)
			data.ParallaxScale = 0.1f;

		// Indicate that material require special handling
		if (notSolid)
			assets.emplace<MaterialNotSolid>(materialId);

		//buffers.Init(dev, data, texDesc);
		return materialId;
	}

	void AssetsStreamer::LoadModelData(GFX::Device& dev, Storage& registry, EID root, const std::string& file, bool loadNames)
	{
		ZE_VALID_EID(root);
		ZE_ASSERT(registry.all_of<Transform>(root), "Entity should contain Transform component!");
		ZE_ASSERT(registry.all_of<TransformGlobal>(root), "Entity should contain TransformGlobal component!");

		Assimp::Importer importer;
		importer.SetPropertyFloat(AI_CONFIG_PP_GSN_MAX_SMOOTHING_ANGLE, 80.0f);
		importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS,
			aiComponent_COLORS | aiComponent_CAMERAS | aiComponent_ANIMATIONS | aiComponent_LIGHTS);
		// aiProcess_FindInstances <- takes a while??
		// aiProcess_GenBoundingBoxes ??? No info
		const aiScene* scene = importer.ReadFile(file,
			aiProcess_ConvertToLeftHanded |
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
			throw ZE_MDL_EXCEPT(error);

		std::filesystem::path filePath(file);
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
			Float4& rotation = registry.get<TransformGlobal>(root).Rotation;
			Math::XMStoreFloat4(&rotation,
				Math::XMQuaternionNormalize(Math::XMQuaternionMultiply(Math::XMQuaternionRotationRollPitchYaw(Math::ToRadians(90.0f), 0.0f, 0.0f),
					Math::XMLoadFloat4(&rotation))));
		}

		// Load geometry
		std::vector<std::pair<EID, U32>> meshes;
		meshes.reserve(scene->mNumMeshes);
		for (U32 i = 0; i < scene->mNumMeshes; ++i)
			meshes.emplace_back(ParseMesh(dev, *scene->mMeshes[i], loadNames), scene->mMeshes[i]->mMaterialIndex);

		// Load materials
		std::vector<EID> materials;
		materials.reserve(scene->mNumMaterials);
		const GFX::Resource::Texture::Schema& texSchema = texSchemaLib.Get(MaterialPBR::TEX_SCHEMA_NAME);
		for (U32 i = 0; i < scene->mNumMaterials; ++i)
			materials.emplace_back(ParseMaterial(dev, *scene->mMaterials[i], texSchema, filePath.remove_filename().string(), loadNames));

		// Load model structure
		ParseNode(*scene->mRootNode, root, meshes, materials, registry, loadNames);
	}
#endif
}