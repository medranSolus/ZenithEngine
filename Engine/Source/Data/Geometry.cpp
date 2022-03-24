#include "Data/Tags.h"
#include "GFX/Vertex.h"
#ifdef _ZE_MODEL_LOADING
#	include "assimp/Importer.hpp"
#	include "assimp/scene.h"
#	include "assimp/postprocess.h"
#endif

namespace ZE::Data
{
#ifdef _ZE_MODEL_LOADING
	EID ParseMesh(const aiMesh& mesh, GFX::Device& dev, Storage& meshRegistry)
	{
		U32* indices = new U32[mesh.mNumFaces * 3];
		GFX::IndexData indexData = { 0, indices };
		for (U32 i = 0; i < mesh.mNumFaces; ++i)
		{
			const aiFace& face = mesh.mFaces[i];
			ZE_ASSERT(face.mNumIndices == 3, "Mesh face is not a triangle!");

			indices[indexData.Count++] = face.mIndices[0];
			indices[indexData.Count++] = face.mIndices[1];
			indices[indexData.Count++] = face.mIndices[2];
		}

		GFX::Vertex* vertices = new GFX::Vertex[mesh.mNumVertices];
		for (U32 i = 0; i < mesh.mNumVertices; ++i)
		{
			GFX::Vertex& vertex = vertices[i];

			vertex.Position = *reinterpret_cast<Float3*>(mesh.mVertices + i);
			ZE_ASSERT(vertex.Position.x == mesh.mVertices[i].x
				&& vertex.Position.y == mesh.mVertices[i].y
				&& vertex.Position.z == mesh.mVertices[i].z,
				"Position data is incorrect, type of aiMesh::mVertices changed!");

			vertex.Normal = *reinterpret_cast<Float3*>(mesh.mNormals + i);
			ZE_ASSERT(vertex.Normal.x == mesh.mNormals[i].x
				&& vertex.Normal.y == mesh.mNormals[i].y
				&& vertex.Normal.z == mesh.mNormals[i].z,
				"Normal data is incorrect, type of aiMesh::mNormals changed!");

			vertex.UV = *reinterpret_cast<Float2*>(mesh.mTextureCoords[0] + i);
			ZE_ASSERT(vertex.UV.x == mesh.mTextureCoords[0][i].x
				&& vertex.UV.y == mesh.mTextureCoords[0][i].y,
				"UV data is incorrect, type of aiMesh::mTextureCoords changed!");

			vertex.Tangent.x = mesh.mTangents[i].x;
			vertex.Tangent.y = mesh.mTangents[i].y;
			vertex.Tangent.z = mesh.mTangents[i].z;
			//vertex.Tangent.w = -1.0f; // TODO: Check somehow
		}

		EID meshId = meshRegistry.create();
		Geometry& geometry = meshRegistry.emplace<Geometry>(meshId);
		geometry.Indices.Init(dev, indexData);
		geometry.Vertices.Init(dev, { mesh.mNumVertices, sizeof(GFX::Vertex), vertices });

		delete[] vertices;
		delete[] indices;
		return meshId;
	}

	EID ParseMaterial(const aiMaterial& material, const GFX::Resource::Texture::Schema& schema,
		const std::string& path, GFX::Device& dev, Storage& materialRegistry)
	{
		EID materialId = materialRegistry.create();
		MaterialPBR& data = materialRegistry.emplace<MaterialPBR>(materialId);
		MaterialBuffersPBR& buffers = materialRegistry.emplace<MaterialBuffersPBR>(materialId);

		GFX::Resource::Texture::PackDesc texDesc;
		texDesc.Init(schema);
		std::vector<GFX::Surface> surfaces;
		aiString texFile;

		// Get diffuse texture
		if (material.GetTexture(aiTextureType_DIFFUSE, 0, &texFile) == aiReturn_SUCCESS)
		{
			surfaces.emplace_back(path + texFile.C_Str());
			texDesc.AddTexture(schema, MaterialPBR::TEX_COLOR_NAME, std::move(surfaces));
			data.Flags |= MaterialPBR::Flag::UseTexture;
		}

		// Get normal map texture
		if (material.GetTexture(aiTextureType_NORMALS, 0, &texFile) == aiReturn_SUCCESS)
		{
			surfaces.emplace_back(path + texFile.C_Str());
			texDesc.AddTexture(schema, MaterialPBR::TEX_NORMAL_NAME, std::move(surfaces));
			data.Flags |= MaterialPBR::Flag::UseNormal;
		}

		// Get specular data
		if (material.GetTexture(aiTextureType_SPECULAR, 0, &texFile) == aiReturn_SUCCESS)
		{
			surfaces.emplace_back(path + texFile.C_Str());
			if (surfaces.back().HasAlpha())
				data.Flags |= MaterialPBR::Flag::UseSpecularPowerAlpha;
			texDesc.AddTexture(schema, MaterialPBR::TEX_SPECULAR_NAME, std::move(surfaces));
			data.Flags |= MaterialPBR::Flag::UseSpecular;
		}

		// Get parallax map texture
		if (material.GetTexture(aiTextureType_HEIGHT, 0, &texFile) == aiReturn_SUCCESS)
		{
			surfaces.emplace_back(path + texFile.C_Str());
			texDesc.AddTexture(schema, MaterialPBR::TEX_HEIGHT_NAME, std::move(surfaces));
			data.Flags |= MaterialPBR::Flag::UseParallax;
		}

		if (material.Get(AI_MATKEY_COLOR_DIFFUSE, reinterpret_cast<aiColor4D&>(data.Color)) != aiReturn_SUCCESS)
			data.Color = { 0.0f, 0.8f, 1.0f };

		if (material.Get(AI_MATKEY_COLOR_SPECULAR, reinterpret_cast<aiColor3D&>(data.Specular)) != aiReturn_SUCCESS)
			data.Specular = { 1.0f, 1.0f, 1.0f };

		if (material.Get(AI_MATKEY_SHININESS_STRENGTH, data.SpecularIntensity) != aiReturn_SUCCESS)
			data.SpecularIntensity = 0.9f;

		if (material.Get(AI_MATKEY_SHININESS, data.SpecularPower) != aiReturn_SUCCESS)
			data.SpecularPower = 0.409f;
		else if (data.SpecularPower > 1.0f)
			data.SpecularPower = static_cast<float>(log(static_cast<double>(data.SpecularPower)) / log(8192.0));

		if (material.Get(AI_MATKEY_BUMPSCALING, data.ParallaxScale) != aiReturn_SUCCESS)
			data.ParallaxScale = 0.1f;

		buffers.Init(dev, data, texDesc);
		return materialId;
	}

	void ParseNode(const aiNode& node, EID currentEntity, const std::vector<std::pair<EID, U32>>& meshes,
		const std::vector<EID>& materials, Storage& registry) noexcept
	{
		TransformGlobal& transform = registry.get<TransformGlobal>(currentEntity);
		const Vector globalRotation = Math::XMLoadFloat4(&transform.Rotation);
		const Vector globalPosition = Math::XMLoadFloat3(&transform.Position);
		const Vector globalScale = Math::XMLoadFloat3(&transform.Scale);

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
				registry.emplace<Transform>(child, Transform({ 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }));
				registry.emplace<TransformGlobal>(child, transform);
				registry.emplace<MeshID>(child, meshes.at(node.mMeshes[i]).first);
				registry.emplace<MaterialID>(child, materials.at(meshes.at(node.mMeshes[i]).second));
			}
		}

		for (U32 i = 0; i < node.mNumChildren; ++i)
		{
			EID child = registry.create();
			registry.emplace<ParentID>(child, currentEntity);

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

			ParseNode(*node.mChildren[i], child, meshes, materials, registry);
		}
	}

	void LoadGeometryFromModel(GFX::Device& dev, GFX::Resource::Texture::Library& textureLib,
		Storage& registry, Storage& resourceRegistry, EID root, const std::string& file)
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
		std::string error = importer.GetErrorString();
		if (!scene || error.size())
			throw ZE_MDL_EXCEPT(std::move(error));

		std::filesystem::path filePath(file);
		// Fix for incorrect format with YZ coords
		bool flipYZ = filePath.extension().string() == ".3ds";
		if (flipYZ)
		{
			float temp = scene->mRootNode->mTransformation.b2;
			scene->mRootNode->mTransformation.b2 = -scene->mRootNode->mTransformation.b3;
			scene->mRootNode->mTransformation.b3 = temp;
			std::swap(scene->mRootNode->mTransformation.c2, scene->mRootNode->mTransformation.c3);
		}

		dev.BeginUploadRegion();
		std::vector<std::pair<EID, U32>> meshes;
		meshes.reserve(scene->mNumMeshes);
		for (U32 i = 0; i < scene->mNumMeshes; ++i)
			meshes.emplace_back(ParseMesh(*scene->mMeshes[i], dev, resourceRegistry), scene->mMeshes[i]->mMaterialIndex);
		dev.StartUpload();

		std::vector<EID> materials;
		materials.reserve(scene->mNumMaterials);
		const GFX::Resource::Texture::Schema& textureSchema = textureLib.Get(MaterialPBR::TEX_SCHEMA_NAME);
		for (U32 i = 0; i < scene->mNumMaterials; ++i)
			materials.emplace_back(ParseMaterial(*scene->mMaterials[i], textureSchema,
				filePath.remove_filename().string(), dev, resourceRegistry));
		dev.StartUpload();
		dev.EndUploadRegion();

		ParseNode(*scene->mRootNode, root, meshes, materials, registry);
	}
#endif
}