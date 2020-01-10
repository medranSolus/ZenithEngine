#define _USE_MATH_DEFINES
#include "Model.h"
#include "GfxResources.h"
#include "Surface.h"
#include "Math.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "ImGui/imgui.h"

namespace GFX::Shape
{
	Model::Node::Node(const std::string & name, std::vector<std::shared_ptr<Mesh>>&& nodeMeshes, const DirectX::FXMMATRIX & nodeTransform) noexcept
		: Object(name), meshes(std::move(nodeMeshes))
	{
		DirectX::XMStoreFloat4x4(&baseTransform, nodeTransform);
		currentTransform = std::make_shared<DirectX::XMFLOAT4X4>();
		currentScaling = std::make_shared<DirectX::XMFLOAT4X4>();
		for (auto & mesh : meshes)
		{
			mesh->SetTransformMatrix(currentTransform);
			mesh->SetScalingMatrix(currentScaling);
		}
	}
	
	void Model::Node::Draw(Graphics & gfx, const DirectX::FXMMATRIX & higherTransform, const DirectX::FXMMATRIX & higherScaling) const noexcept
	{
		const DirectX::XMMATRIX transformMatrix = DirectX::XMLoadFloat4x4(transform.get()) *
			DirectX::XMLoadFloat4x4(&baseTransform) * higherTransform;
		const DirectX::XMMATRIX scalingMatrix = DirectX::XMLoadFloat4x4(scaling.get()) * higherScaling;
		DirectX::XMStoreFloat4x4(currentTransform.get(), transformMatrix);
		DirectX::XMStoreFloat4x4(currentScaling.get(), scalingMatrix);
		for (const auto & mesh : meshes)
			mesh->Draw(gfx);
		for (const auto & child : children)
			child->Draw(gfx, transformMatrix, scalingMatrix);
	}

	void Model::Node::ShowTree(unsigned long long & nodeId, unsigned long long & selectedId, Node * & selectedNode) const noexcept
	{
		const unsigned long long currentNode = nodeId++;
		const bool expanded = ImGui::TreeNodeEx((void*)currentNode,
			ImGuiTreeNodeFlags_OpenOnArrow |
			(children.size() ? 0 : ImGuiTreeNodeFlags_Leaf) |
			(currentNode == selectedId ? ImGuiTreeNodeFlags_Selected : 0), name.c_str());
		if (ImGui::IsItemClicked())
		{
			selectedId = currentNode;
			selectedNode = const_cast<Node*>(this);
		}
		if (expanded)
		{
			for (auto & child : children)
				child->ShowTree(nodeId, selectedId, selectedNode);
			ImGui::TreePop();
		}
	}

	void Model::Window::Show() noexcept
	{
		ImGui::Columns(2);
		ImGui::BeginChild("##scroll", ImVec2(0.0f, 231.5f), false, ImGuiWindowFlags_HorizontalScrollbar);
		unsigned long long startId = 0;
		parent->root->ShowTree(startId, selectedId, selectedNode);
		ImGui::EndChild();
		ImGui::NextColumn();
		ImGui::NewLine();
		selectedNode->Object::ShowWindow();
		ImGui::Columns(1);
	}

	unsigned long long Model::modelCount = 0U;

	std::shared_ptr<Mesh> Model::ParseMesh(Graphics & gfx, const aiMesh & mesh, const aiMaterial *const * materials)
	{
		BasicType::VertexDataBuffer vertexBuffer(std::move(BasicType::VertexLayout{}
			.Append(VertexAttribute::Normal)
			.Append(VertexAttribute::Texture2D)));

		vertexBuffer.Reserve(mesh.mNumVertices);
		for (unsigned int i = 0; i < mesh.mNumVertices; ++i)
			vertexBuffer.EmplaceBack(*reinterpret_cast<DirectX::XMFLOAT3*>(&mesh.mVertices[i]),
				*reinterpret_cast<DirectX::XMFLOAT3*>(&mesh.mNormals[i]),
				*reinterpret_cast<DirectX::XMFLOAT2*>(&mesh.mTextureCoords[0][i]));

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

		auto vertexShader = std::make_unique<Resource::VertexShader>(gfx, L"TexturePhongVS.cso");
		auto bytecodeVS = vertexShader->GetBytecode();
		binds.emplace_back(std::move(vertexShader));

		binds.emplace_back(std::make_unique<Resource::InputLayout>(gfx, vertexBuffer.GetLayout().GetDXLayout(), bytecodeVS));
		binds.emplace_back(std::make_unique<Resource::VertexBuffer>(gfx, std::move(vertexBuffer)));

		if (mesh.mMaterialIndex >= 0)
		{
			const aiMaterial & material = *materials[mesh.mMaterialIndex];
			aiString texFile;
			material.GetTexture(aiTextureType_DIFFUSE, 0, &texFile);
			binds.emplace_back(std::make_unique<Resource::Texture>(gfx, Surface("Models/nanosuit/" + std::string(texFile.C_Str()))));
			if (material.GetTexture(aiTextureType_SPECULAR, 0, &texFile) == aiReturn_SUCCESS)
			{
				binds.emplace_back(std::make_unique<Resource::Texture>(gfx, Surface("Models/nanosuit/" + std::string(texFile.C_Str())), 1U));
				binds.emplace_back(std::make_unique<Resource::PixelShader>(gfx, L"TextureSpecularPhongPS.cso"));
			}
			else
			{
				Resource::ObjectConstantBuffer buffer;
				material.Get(AI_MATKEY_SHININESS, buffer.specularPower);
				binds.emplace_back(std::make_unique<Resource::PixelShader>(gfx, L"TexturePhongPS.cso"));
				buffer.specularIntensity = 0.9f;
				binds.emplace_back(std::make_unique<Resource::ConstantPixelBuffer<Resource::ObjectConstantBuffer>>(gfx, buffer, 1U));
			}
			binds.emplace_back(std::make_unique<Resource::Sampler>(gfx));
		}
		
		return std::make_shared<Mesh>(gfx, std::move(binds));
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
		: name(modelName)
	{
		Assimp::Importer importer;
		const aiScene * scene = importer.ReadFile(file, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_ConvertToLeftHanded | aiProcess_GenSmoothNormals);
		if (!scene)
			throw ModelException(__LINE__, __FILE__, importer.GetErrorString());

		meshes.reserve(scene->mNumMeshes);
		name = modelName == "Model_" ? "Model_" + modelCount++ : modelName;
		for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
			meshes.emplace_back(ParseMesh(gfx, *scene->mMeshes[i], scene->mMaterials));
		root = ParseNode(*scene->mRootNode);
		root->SetScale(scale);
		root->SetPos(position);
		window = std::make_unique<Window>(const_cast<Model*>(this));
	}

	const char * Model::ModelException::what() const noexcept
	{
		std::ostringstream stream;
		stream << BasicException::what()
			<< "\n[Assimp Error] " << error;
		whatBuffer = stream.str();
		return whatBuffer.c_str();
	}
}
