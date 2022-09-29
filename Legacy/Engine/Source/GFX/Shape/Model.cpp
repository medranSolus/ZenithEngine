#include "GFX/Shape/Model.h"
#include "GFX/Pipeline/TechniqueFactory.h"
#include "Exception/ModelException.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include <filesystem>

namespace ZE::GFX::Shape
{
	std::shared_ptr<Mesh> Model::ParseMesh(Graphics& gfx, Pipeline::RenderGraph& graph, const std::string& path, aiMesh& mesh)
	{
		// Maybe layout code needed too, TODO: Check this
		std::string meshID = std::to_string(mesh.mNumFaces) + std::string(mesh.mName.C_Str()) + std::to_string(mesh.mNumVertices) + "#";
		std::vector<U32> indices;
		if (Resource::IndexBuffer::NotStored(meshID))
		{
			indices.reserve(static_cast<U64>(mesh.mNumFaces) * 3);
			for (U32 i = 0; i < mesh.mNumFaces; ++i)
			{
				const auto& face = mesh.mFaces[i];
				assert(face.mNumIndices == 3);
				indices.emplace_back(face.mIndices[0]);
				indices.emplace_back(face.mIndices[1]);
				indices.emplace_back(face.mIndices[2]);
			}
		}
		auto indexBuffer = Resource::IndexBuffer::Get(gfx, meshID, std::move(indices));
		auto material = materials.at(mesh.mMaterialIndex);
		auto vertexLayout = material->GerVertexLayout();
		meshID += vertexLayout->GetLayoutCode();

		GfxResPtr<Resource::VertexBuffer> vertexBuffer;
		if (Resource::VertexBuffer::NotStored(meshID))
			vertexBuffer = Resource::VertexBuffer::Get(gfx, meshID, { vertexLayout, mesh });
		else
			vertexBuffer = Resource::VertexBuffer::Get(gfx, meshID, { vertexLayout });

		std::vector<Pipeline::Technique> techniques;
		techniques.reserve(3);
		techniques.emplace_back(Pipeline::TechniqueFactory::MakeShadowMap(gfx, graph, material));
		techniques.emplace_back(Pipeline::TechniqueFactory::MakeLambertian(gfx, graph, std::move(material)));
		techniques.emplace_back(Pipeline::TechniqueFactory::MakeOutlineBlur(gfx, graph, meshID, std::move(vertexLayout)));

		return std::make_shared<Mesh>(gfx, *name, std::move(indexBuffer), std::move(vertexBuffer), std::move(techniques));
	}

	std::unique_ptr<ModelNode> Model::ParseNode(const aiNode& node, U64& id)
	{
		std::vector<std::shared_ptr<Mesh>> currentMeshes;
		currentMeshes.reserve(node.mNumMeshes);
		for (U32 i = 0; i < node.mNumMeshes; ++i)
			currentMeshes.emplace_back(meshes.at(node.mMeshes[i]));

		std::unique_ptr<ModelNode> currentNode = std::make_unique<ModelNode>(id, node.mName.C_Str(), std::move(currentMeshes),
			Math::XMMatrixTranspose(Math::XMLoadFloat4x4(reinterpret_cast<const Float4x4*>(&node.mTransformation))));

		currentNode->ReserveChildren(node.mNumChildren);
		for (U32 i = 0; i < node.mNumChildren; ++i)
			currentNode->AddChild(ParseNode(*node.mChildren[i], ++id));
		return currentNode;
	}

	Model::Model(Graphics& gfx, Pipeline::RenderGraph& graph, const std::string& file, const ModelParams& params)
		: name(std::make_unique<std::string>(params.name))
	{
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

		meshes.reserve(scene->mNumMeshes);
		materials.reserve(scene->mNumMaterials);
		std::filesystem::path filePath(file);
		bool flipYZ = filePath.extension().string() == ".3ds";
		if (flipYZ) // Fix for incorrect format with YZ coords
		{
			float temp = scene->mRootNode->mTransformation.b2;
			scene->mRootNode->mTransformation.b2 = -scene->mRootNode->mTransformation.b3;
			scene->mRootNode->mTransformation.b3 = temp;
			std::swap(scene->mRootNode->mTransformation.c2, scene->mRootNode->mTransformation.c3);
		}

		std::string path = filePath.remove_filename().string();
		for (U32 i = 0; i < scene->mNumMaterials; ++i)
			materials.emplace_back(std::make_shared<Visual::Material>(gfx, *scene->mMaterials[i], path));

		for (U32 i = 0; i < scene->mNumMeshes; ++i)
			meshes.emplace_back(ParseMesh(gfx, graph, path, *scene->mMeshes[i]));
		U64 startID = 0;
		root = ParseNode(*scene->mRootNode, startID);
		root->SetScale(params.scale);
		root->SetPos(params.position);
		if (flipYZ)
		{
			root->SetAngle(Math::XMQuaternionNormalize(Math::XMQuaternionMultiply(Math::XMLoadFloat4(&params.rotation),
				Math::XMQuaternionRotationNormal({ 1.0f, 0.0f, 0.0f, 0.0f }, M_PI_2))));
		}
		else
			root->SetAngle(params.rotation);
	}

	void Model::SetOutline() noexcept
	{
		for (auto& mesh : meshes)
			mesh->SetOutline();
		isOutline = true;
	}

	void Model::DisableOutline() noexcept
	{
		for (auto& mesh : meshes)
			mesh->DisableOutline();
		isOutline = false;
	}
}