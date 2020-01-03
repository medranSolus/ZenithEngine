#include "Model.h"
#include "GfxResources.h"
#include "Math.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include <random>

namespace GFX::Shape
{
	unsigned long long Model::modelCount = 0U;

	Model::Node::Node(const std::string & name, std::vector<std::shared_ptr<Mesh>> && meshes, const DirectX::FXMMATRIX & nodeTransform) noexcept
		: name(name), meshes(std::move(meshes))
	{
		DirectX::XMStoreFloat4x4(&transform, nodeTransform);
	}

	void Model::Node::Draw(Graphics & gfx, const DirectX::FXMMATRIX & higherTransform) const noexcept
	{
		const DirectX::XMMATRIX currentTransform = DirectX::XMLoadFloat4x4(&transform) * higherTransform;
		for (const auto & mesh : meshes)
			mesh->Draw(gfx, currentTransform);
		for (const auto & child : children)
			child->Draw(gfx, currentTransform);
	}

	std::unique_ptr<Model::Node> Model::ParseNode(const aiNode & node) noexcept
	{
		std::vector<std::shared_ptr<Mesh>> currentMeshes;
		currentMeshes.reserve(node.mNumMeshes);
		for (unsigned int i = 0; i < node.mNumMeshes; ++i)
			currentMeshes.emplace_back(meshes.at(node.mMeshes[i]));

		std::unique_ptr<Node> currentNode = std::make_unique<Node>(node.mName.C_Str(), std::move(currentMeshes), 
			DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(reinterpret_cast<const DirectX::XMFLOAT4X4*>(&node.mTransformation))));

		currentNode->ReserveChildren(node.mNumChildren);
		for (unsigned int i = 0; i < node.mNumChildren; ++i)
			currentNode->AddChild(ParseNode(*node.mChildren[i]));
		return currentNode;
	}

	Model::Model(Graphics & gfx, const std::string & file, const DirectX::XMFLOAT3 & position, const std::string & modelName, float scale)
		: Object(position, modelName), scale(scale)
	{
		Assimp::Importer importer;
		const aiScene * model = importer.ReadFile(file, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_ConvertToLeftHanded | aiProcess_GenSmoothNormals);
		meshes.reserve(model->mNumMeshes);
		name = modelName == "Model_" ? "Model_" + modelCount++ : modelName;
		for (unsigned int i = 0; i < model->mNumMeshes; ++i)
			meshes.emplace_back(ParseMesh(gfx, *model->mMeshes[i]));
		root = ParseNode(*model->mRootNode);
	}

	std::shared_ptr<Mesh> Model::ParseMesh(Graphics & gfx, const aiMesh & mesh)
	{
		BasicType::VertexDataBuffer vertexBuffer(std::move(BasicType::VertexLayout{}.Append(VertexAttribute::Normal)));

		vertexBuffer.Reserve(mesh.mNumVertices);
		for (unsigned int i = 0; i < mesh.mNumVertices; ++i)
			vertexBuffer.EmplaceBack(*reinterpret_cast<DirectX::XMFLOAT3*>(&mesh.mVertices[i]),
				*reinterpret_cast<DirectX::XMFLOAT3*>(&mesh.mNormals[i]));

		std::vector<unsigned int> indices;
		indices.reserve(mesh.mNumFaces * 3);
		for (unsigned int i = 0; i < mesh.mNumFaces; ++i)
		{
			const auto & face = mesh.mFaces[i];
			assert(face.mNumIndices == 3);
			indices.emplace_back(face.mIndices[0]);
			indices.emplace_back(face.mIndices[1]);
			indices.emplace_back(face.mIndices[2]);
		}

		std::vector<std::unique_ptr<Resource::IBindable>> binds;
		binds.reserve(6U);
		binds.emplace_back(std::make_unique<Resource::IndexBuffer>(gfx, std::move(indices)));

		auto vertexShader = std::make_unique<Resource::VertexShader>(gfx, L"PhongVS.cso");
		auto bytecodeVS = vertexShader->GetBytecode();
		binds.emplace_back(std::move(vertexShader));
		binds.emplace_back(std::make_unique<Resource::PixelShader>(gfx, L"PhongPS.cso"));

		binds.emplace_back(std::make_unique<Resource::InputLayout>(gfx, vertexBuffer.GetLayout().GetDXLayout(), bytecodeVS));
		binds.emplace_back(std::make_unique<Resource::VertexBuffer>(gfx, std::move(vertexBuffer)));

		std::mt19937_64 engine(std::random_device{}());
		Resource::ObjectConstantBuffer buffer;
		buffer.materialColor = randColor(engine);
		buffer.specularIntensity = 0.9f;
		buffer.specularPower = 40.0f;
		binds.emplace_back(std::make_unique<Resource::ConstantPixelBuffer<Resource::ObjectConstantBuffer>>(gfx, buffer, 1U));

		return std::make_shared<Mesh>(gfx, std::move(binds));
	}
	
	void Model::Draw(Graphics & gfx) const noexcept
	{
		root->Draw(gfx, DirectX::XMMatrixScaling(scale, scale, scale) *
			DirectX::XMMatrixRotationRollPitchYawFromVector(DirectX::XMLoadFloat3(&angle)) *
			DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&pos)));
	}
}
