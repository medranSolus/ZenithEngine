#define _USE_MATH_DEFINES
#include "Model.h"
#include "GfxResources.h"
#include "Surface.h"
#include "Math.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "ImGui/imgui.h"
#include <filesystem>

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

	std::shared_ptr<Mesh> Model::ParseMesh(Graphics & gfx, const std::string & path, const aiMesh & mesh, aiMaterial *const * materials)
	{
		std::vector<std::shared_ptr<Resource::IBindable>> binds;
		binds.reserve(8U);

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
		std::string meshID = std::string(mesh.mName.C_Str()) + std::to_string(indices.size()) + std::to_string(mesh.mNumVertices);
		// Maybe layout code needed too, TODO: Check this
		binds.emplace_back(Resource::IndexBuffer::Get(gfx, meshID, std::move(indices)));

		std::shared_ptr<BasicType::VertexLayout> layout = std::make_shared<BasicType::VertexLayout>();
		bool normals = mesh.HasNormals();
		bool textureCoord = mesh.HasTextureCoords(0);

		std::shared_ptr<Resource::VertexShader> vertexShader = nullptr;
		if (normals)
		{
			bool noTexture = true;
			Resource::PhongPixelBuffer buffer;
			if (textureCoord && mesh.mMaterialIndex >= 0)
			{
				aiMaterial & material = *materials[mesh.mMaterialIndex];
				aiString texFile;
				if (material.GetTexture(aiTextureType_DIFFUSE, 0, &texFile) == aiReturn_SUCCESS)
				{
					noTexture = false;
					vertexShader = Resource::VertexShader::Get(gfx, "TexturePhongVS.cso");
					binds.emplace_back(Resource::Sampler::Get(gfx));
					
					binds.emplace_back(Resource::Texture::Get(gfx, path + std::string(texFile.C_Str())));
					if (material.GetTexture(aiTextureType_SPECULAR, 0, &texFile) == aiReturn_SUCCESS)
					{
						binds.emplace_back(Resource::Texture::Get(gfx, path + std::string(texFile.C_Str()), 1U));
						binds.emplace_back(Resource::PixelShader::Get(gfx, "TextureSpecularPhongPS.cso"));
					}
					else
					{
						binds.emplace_back(Resource::PixelShader::Get(gfx, "TexturePhongPS.cso"));
						Resource::TexPhongPixelBuffer buffer;
						material.Get(AI_MATKEY_SHININESS, buffer.specularPower);
						if (material.Get(AI_MATKEY_SHININESS_STRENGTH, buffer.specularIntensity) != aiReturn_SUCCESS)
							buffer.specularIntensity = 0.9f;
						// Maybe path needed too, TOD: Check this
						binds.emplace_back(Resource::ConstantPixelBuffer<Resource::TexPhongPixelBuffer>::Get(gfx, material.GetName().C_Str(), buffer, 1U));
					}
				}
				else
				{
					textureCoord = false;
					if (material.Get(AI_MATKEY_SHININESS, buffer.specularPower) != aiReturn_SUCCESS)
						buffer.specularPower = 40.0f;
					if (material.Get(AI_MATKEY_SHININESS_STRENGTH, buffer.specularIntensity) != aiReturn_SUCCESS)
						buffer.specularIntensity = 0.9f;
				}
			}
			else
				buffer.specularIntensity = 0.9f;
			if (noTexture)
			{
				std::mt19937_64 eng(std::random_device{}());
				buffer.materialColor = randColor(eng);
				if (buffer.specularPower <= FLT_EPSILON)
					buffer.specularPower = 40.0f;
				vertexShader = Resource::VertexShader::Get(gfx, "PhongVS.cso");
				binds.emplace_back(Resource::PixelShader::Get(gfx, "PhongPS.cso"));
				binds.emplace_back(Resource::ConstantPixelBuffer<Resource::PhongPixelBuffer>::Get(gfx, path + meshID, buffer, 1U));
			}
		}
		else
		{
			vertexShader = Resource::VertexShader::Get(gfx, "SolidVS.cso");
			binds.emplace_back(Resource::PixelShader::Get(gfx, "SolidPS.cso"));
		}

		if (normals)
		{
			layout->Append(VertexAttribute::Normal);
			if (textureCoord)
				layout->Append(VertexAttribute::Texture2D);
		}

		BasicType::VertexDataBuffer vertexBuffer(layout, mesh.mNumVertices);
		for (unsigned int i = 0; i < mesh.mNumVertices; ++i)
		{
			vertexBuffer[i].SetByIndex(0, *reinterpret_cast<DirectX::XMFLOAT3*>(&mesh.mVertices[i]));
			if (normals)
			{
				vertexBuffer[i].Get<VertexAttribute::Normal>() = *reinterpret_cast<DirectX::XMFLOAT3*>(&mesh.mNormals[i]);
				if (textureCoord)
					vertexBuffer[i].Get<VertexAttribute::Texture2D>() = *reinterpret_cast<DirectX::XMFLOAT2*>(&mesh.mTextureCoords[0][i]);
			}
		}
		binds.emplace_back(Resource::VertexBuffer::Get(gfx, meshID + layout->GetLayoutCode(), std::move(vertexBuffer)));

		auto bytecodeVS = vertexShader->GetBytecode();
		binds.emplace_back(Resource::InputLayout::Get(gfx, layout, bytecodeVS));
		binds.emplace_back(vertexShader);

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
		const aiScene * scene = importer.ReadFile(file, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices |
			aiProcess_ConvertToLeftHanded | aiProcess_GenSmoothNormals | aiProcess_GenUVCoords);
		if (!scene)
			throw ModelException(__LINE__, __FILE__, importer.GetErrorString());

		meshes.reserve(scene->mNumMeshes);
		name = modelName == "Model_" ? "Model_" + modelCount++ : modelName;
		std::string path = std::filesystem::path(file).remove_filename().string();
		for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
			meshes.emplace_back(ParseMesh(gfx, path, *scene->mMeshes[i], scene->mMaterials));
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
