#include "Model.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include <filesystem>

namespace GFX::Shape
{
	std::shared_ptr<Mesh> Model::ParseMesh(Graphics& gfx, const std::string& path, aiMesh& mesh, std::vector<std::shared_ptr<Visual::Material>>& materials)
	{
		// Maybe layout code needed too, TODO: Check this
		std::string meshID = std::to_string(mesh.mNumFaces) + std::string(mesh.mName.C_Str()) + std::to_string(mesh.mNumVertices) + "#";
		std::vector<unsigned int> indices;
		if (Resource::IndexBuffer::NotStored(meshID))
		{
			indices.reserve(static_cast<size_t>(mesh.mNumFaces) * 3);
			for (unsigned int i = 0; i < mesh.mNumFaces; ++i)
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

		std::shared_ptr<Resource::VertexBuffer> vertexBuffer = nullptr;
		if (Resource::VertexBuffer::NotStored(meshID))
			vertexBuffer = Resource::VertexBuffer::Get(gfx, meshID, { std::move(vertexLayout), mesh });
		else
			vertexBuffer = Resource::VertexBuffer::Get(gfx, meshID, { std::move(vertexLayout) });

		std::vector<std::shared_ptr<Pipeline::Technique>> techniques;
		techniques.reserve(2);
		techniques.emplace_back(std::make_shared<Pipeline::Technique>("Phong"));
		techniques.back()->AddStep({ 0, std::move(material) });

		return std::make_shared<Mesh>(gfx, std::move(indexBuffer), std::move(vertexBuffer), std::move(techniques));
	}

	std::unique_ptr<ModelNode> Model::ParseNode(const aiNode& node, unsigned long long& id) noexcept(!IS_DEBUG)
	{
		std::vector<std::shared_ptr<Mesh>> currentMeshes;
		currentMeshes.reserve(node.mNumMeshes);
		for (unsigned int i = 0; i < node.mNumMeshes; ++i)
			currentMeshes.emplace_back(meshes.at(node.mMeshes[i]));

		std::unique_ptr<ModelNode> currentNode = std::make_unique<ModelNode>(id, node.mName.C_Str(), std::move(currentMeshes),
			DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(reinterpret_cast<const DirectX::XMFLOAT4X4*>(&node.mTransformation))));

		currentNode->ReserveChildren(node.mNumChildren);
		for (unsigned int i = 0; i < node.mNumChildren; ++i)
			currentNode->AddChild(ParseNode(*node.mChildren[i], ++id));
		return currentNode;
	}

	Model::Model(Graphics& gfx, const std::string& file, const DirectX::XMFLOAT3& position, const std::string& modelName, float scale)
		: name(modelName)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(file, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices |
			aiProcess_ConvertToLeftHanded | aiProcess_GenSmoothNormals | aiProcess_GenUVCoords | aiProcess_CalcTangentSpace);
		if (!scene)
			throw ModelException(__LINE__, __FILE__, importer.GetErrorString());

		meshes.reserve(scene->mNumMeshes);
		materials.reserve(scene->mNumMaterials);
		std::string path = std::filesystem::path(file).remove_filename().string();
		for (unsigned int i = 0; i < scene->mNumMaterials; ++i)
			materials.emplace_back(std::make_shared<Visual::Material>(gfx, *scene->mMaterials[i], path));
		for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
			meshes.emplace_back(ParseMesh(gfx, path, *scene->mMeshes[i], materials));
		unsigned long long startID = 0ULL;
		root = ParseNode(*scene->mRootNode, startID);
		root->SetScale(scale);
		root->SetPos(position);
	}

	const char* Model::ModelException::what() const noexcept
	{
		std::ostringstream stream;
		stream << BasicException::what()
			<< "\n[Assimp Error] " << error;
		whatBuffer = stream.str();
		return whatBuffer.c_str();
	}
}