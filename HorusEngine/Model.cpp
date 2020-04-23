#include "Model.h"
#include "GfxResources.h"
#include "Surface.h"
#include "Math.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "ImGui/imgui.h"
#include <filesystem>
#include <thread>
#include <array>

namespace GFX::Shape
{
	Model::Node::Node(const std::string& name, std::vector<std::shared_ptr<Mesh>>&& nodeMeshes, const DirectX::FXMMATRIX& nodeTransform) noexcept
		: Object(name), meshes(std::move(nodeMeshes))
	{
		DirectX::XMStoreFloat4x4(&baseTransform, nodeTransform);
		currentTransform = std::make_shared<DirectX::XMFLOAT4X4>();
		for (auto& mesh : meshes)
			mesh->SetTransformMatrix(currentTransform);
	}

	void Model::Node::Submit(Pipeline::RenderCommander& renderer, const DirectX::FXMMATRIX& higherTransform) noexcept(!IS_DEBUG)
	{
		if (visible)
		{
			const DirectX::XMMATRIX transformMatrix = DirectX::XMLoadFloat4x4(transform.get()) *
				DirectX::XMLoadFloat4x4(&baseTransform) * higherTransform;
			DirectX::XMStoreFloat4x4(currentTransform.get(), transformMatrix);
			for (const auto& mesh : meshes)
				mesh->Submit(renderer);
			for (const auto& child : children)
				child->Submit(renderer, transformMatrix);
		}
	}

	void Model::Node::ShowTree(unsigned long long& nodeId, unsigned long long& selectedId, Node*& selectedNode) const noexcept
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
			for (auto& child : children)
				child->ShowTree(nodeId, selectedId, selectedNode);
			ImGui::TreePop();
		}
	}

	void Model::Node::ShowWindow(Graphics& gfx) noexcept
	{
		Object::ShowWindow(gfx);
		bool meshOnly = isMesh;
		ImGui::Checkbox("Mesh-only", &meshOnly);
		if (isMesh != meshOnly)
			SetMesh(gfx, meshOnly);
		ImGui::Checkbox("Object visible", &visible);
	}

	void Model::Node::SetMesh(Graphics& gfx, bool meshOnly) noexcept
	{
		if (isMesh != meshOnly)
		{
			isMesh = meshOnly;
			if (isMesh)
				for (auto& nodeMesh : meshes)
					nodeMesh->SetTopologyMesh(gfx);
			else
				for (auto& nodeMesh : meshes)
					nodeMesh->SetTopologyPlain(gfx);
		}
		for (auto& node : children)
			node->SetMesh(gfx, meshOnly);
	}

	void Model::Window::Show(Graphics& gfx) noexcept
	{
		ImGui::Columns(2);
		ImGui::BeginChild("##scroll", ImVec2(0.0f, 231.5f), false, ImGuiWindowFlags_HorizontalScrollbar);
		unsigned long long startId = 0;
		parent->root->ShowTree(startId, selectedId, selectedNode);
		ImGui::EndChild();
		ImGui::NextColumn();
		ImGui::NewLine();
		selectedNode->ShowWindow(gfx);
		ImGui::Columns(1);
	}

	std::shared_ptr<Mesh> Model::ParseMesh(Graphics& gfx, const std::string& path, aiMesh& mesh, aiMaterial* const* materials)
	{
		std::vector<std::shared_ptr<Resource::IBindable>> binds;
		binds.reserve(8U);

		// Get IndexBuffer
		const size_t indiciesSize = static_cast<size_t>(mesh.mNumFaces) * 3;
		std::vector<unsigned int> indices;
		// Maybe layout code needed too, TODO: Check this
		std::string meshID = std::string(mesh.mName.C_Str()) + std::to_string(indiciesSize) + std::to_string(mesh.mNumVertices);
		if (Resource::IndexBuffer::NotStored(meshID))
		{
			indices.reserve(indiciesSize);
			for (unsigned int i = 0; i < mesh.mNumFaces; ++i)
			{
				const auto& face = mesh.mFaces[i];
				assert(face.mNumIndices == 3);
				indices.emplace_back(face.mIndices[0]);
				indices.emplace_back(face.mIndices[1]);
				indices.emplace_back(face.mIndices[2]);
			}
		}
		binds.emplace_back(Resource::IndexBuffer::Get(gfx, meshID, std::move(indices)));

		bool hasAlpha = false;
		bool normals = mesh.HasNormals();
		bool textureCoord = mesh.HasTextureCoords(0);
		bool normalMap = false;
		std::shared_ptr<Resource::VertexShader> vertexShader = nullptr;
		if (normals)
		{
			bool noTexture = true;
			Data::CBuffer::DCBLayout cbufferLayout;
			cbufferLayout.Add(DCBElementType::Float, "specularIntensity");
			cbufferLayout.Add(DCBElementType::Float, "specularPower");
			std::shared_ptr<Data::CBuffer::DynamicCBuffer> buffer = nullptr;
			if (textureCoord && mesh.mMaterialIndex >= 0)
			{
				aiMaterial& material = *materials[mesh.mMaterialIndex];
				aiString texFile;
				if (material.GetTexture(aiTextureType_DIFFUSE, 0, &texFile) == aiReturn_SUCCESS)
				{
					noTexture = false;
					binds.emplace_back(Resource::Sampler::Get(gfx));
					std::shared_ptr<Resource::Texture> diffuseTexture = Resource::Texture::Get(gfx, path + std::string(texFile.C_Str()), 0U, true);
					hasAlpha = diffuseTexture->HasAlpha();
					binds.emplace_back(std::move(diffuseTexture));

					normalMap = mesh.HasTangentsAndBitangents() && material.GetTexture(aiTextureType_NORMALS, 0, &texFile) == aiReturn_SUCCESS;
					if (normalMap)
					{
						vertexShader = Resource::VertexShader::Get(gfx, "PhongVSTextureNormal.cso");
						binds.emplace_back(Resource::Texture::Get(gfx, path + std::string(texFile.C_Str()), 1U));
					}
					else
						vertexShader = Resource::VertexShader::Get(gfx, "PhongVSTexture.cso");

					if (material.GetTexture(aiTextureType_SPECULAR, 0, &texFile) == aiReturn_SUCCESS)
					{
						// TODO: Check texture HasAlpha, otherwise get alpha from material and send via cbuff.
						binds.emplace_back(Resource::Texture::Get(gfx, path + std::string(texFile.C_Str()), 2U));
						if (normalMap)
							binds.emplace_back(Resource::PixelShader::Get(gfx, "PhongPSTextureNormalSpecular.cso"));
						else
							binds.emplace_back(Resource::PixelShader::Get(gfx, "PhongPSTextureSpecular.cso"));
					}
					else
					{
						if (normalMap)
							binds.emplace_back(Resource::PixelShader::Get(gfx, "PhongPSTextureNormal.cso"));
						else
							binds.emplace_back(Resource::PixelShader::Get(gfx, "PhongPSTexture.cso"));
						buffer = std::make_shared<Data::CBuffer::DynamicCBuffer>(std::move(cbufferLayout));
						material.Get(AI_MATKEY_SHININESS, static_cast<float&>((*buffer)["specularPower"]));
						if (material.Get(AI_MATKEY_SHININESS_STRENGTH, static_cast<float&>((*buffer)["specularIntensity"])) != aiReturn_SUCCESS)
							(*buffer)["specularIntensity"] = 0.9f;
						// Maybe path needed too, TODO: Check this
						binds.emplace_back(Resource::ConstBufferExPixel::Get(gfx, material.GetName().C_Str(), buffer->GetRootElement(), 1U, buffer.get()));
					}
				}
				else
				{
					cbufferLayout.Add(DCBElementType::Color4, "materialColor");
					buffer = std::make_shared<Data::CBuffer::DynamicCBuffer>(std::move(cbufferLayout));
					textureCoord = false;
					if (material.Get(AI_MATKEY_SHININESS, static_cast<float&>((*buffer)["specularPower"])) != aiReturn_SUCCESS)
						(*buffer)["specularPower"] = 40.0f;
					if (material.Get(AI_MATKEY_SHININESS_STRENGTH, static_cast<float&>((*buffer)["specularIntensity"])) != aiReturn_SUCCESS)
						(*buffer)["specularIntensity"] = 0.9f;
				}
			}
			else
			{
				cbufferLayout.Add(DCBElementType::Color4, "materialColor");
				buffer = std::make_shared<Data::CBuffer::DynamicCBuffer>(std::move(cbufferLayout));
				(*buffer)["specularIntensity"] = 0.9f;
			}
			if (noTexture)
			{
				std::mt19937_64 eng(std::random_device{}());
				(*buffer)["materialColor"] = randColor(eng);
				if (static_cast<float>((*buffer)["specularPower"]) <= FLT_EPSILON)
					(*buffer)["specularPower"] = 40.0f;
				vertexShader = Resource::VertexShader::Get(gfx, "PhongVS.cso");
				binds.emplace_back(Resource::PixelShader::Get(gfx, "PhongPS.cso"));
				binds.emplace_back(Resource::ConstBufferExPixel::Get(gfx, path + meshID, buffer->GetRootElement(), 1U, buffer.get()));
			}
		}
		else
		{
			vertexShader = Resource::VertexShader::Get(gfx, "SolidVS.cso");
			binds.emplace_back(Resource::PixelShader::Get(gfx, "SolidPS.cso"));
		}

		std::shared_ptr<Data::VertexLayout> layout = std::make_shared<Data::VertexLayout>();
		if (normals)
		{
			layout->Append(VertexAttribute::Normal);
			if (textureCoord)
			{
				layout->Append(VertexAttribute::Texture2D);
				if (normalMap)
				{
					layout->Append(VertexAttribute::Tangent);
					layout->Append(VertexAttribute::Bitangent);
				}
			}
		}

		meshID += layout->GetLayoutCode();
		Data::VertexBufferData vertexBuffer(layout);
		if (Resource::VertexBuffer::NotStored(meshID))
		{
			vertexBuffer.Resize(mesh.mNumVertices);
			auto loadVertexAttrib = [&vertexBuffer, &mesh]<typename T>(size_t index, aiVector3D * attribs, const T & vectorType) -> void
			{
				for (unsigned int i = 0; i < mesh.mNumVertices; ++i)
					vertexBuffer[i].SetByIndex(index, *reinterpret_cast<T*>(&attribs[i]));
			};
			std::array<std::thread*, 5> vectorAttribThreads{ nullptr };
			DirectX::XMFLOAT3 vec3;
			vectorAttribThreads[0] = new std::thread([&loadVertexAttrib, &mesh, &vec3]() { loadVertexAttrib(0, mesh.mVertices, vec3); });
			if (normals)
			{
				vectorAttribThreads[1] = new std::thread([&loadVertexAttrib, &mesh, &vec3]() { loadVertexAttrib(1, mesh.mNormals, vec3); });
				if (textureCoord)
				{
					DirectX::XMFLOAT2 vec2;
					vectorAttribThreads[2] = new std::thread([&loadVertexAttrib, &mesh, &vec2]() { loadVertexAttrib(2, mesh.mTextureCoords[0], vec2); });
					if (normalMap)
					{
						vectorAttribThreads[3] = new std::thread([&loadVertexAttrib, &mesh, &vec3]() { loadVertexAttrib(3, mesh.mTangents, vec3); });
						vectorAttribThreads[4] = new std::thread([&loadVertexAttrib, &mesh, &vec3]() { loadVertexAttrib(4, mesh.mBitangents, vec3); });
					}
				}
			}

			for (auto& thread : vectorAttribThreads)
				if (thread)
				{
					thread->join();
					delete thread;
				}
		}
		binds.emplace_back(Resource::VertexBuffer::Get(gfx, meshID, std::move(vertexBuffer)));

		binds.emplace_back(Resource::InputLayout::Get(gfx, std::move(layout), vertexShader->GetBytecode()));
		binds.emplace_back(std::move(vertexShader));
		binds.emplace_back(Resource::Blender::Get(gfx, hasAlpha));
		//binds.emplace_back(Resource::Rasterizer::Get(gfx, hasAlpha));

		return std::make_shared<Mesh>(gfx, std::move(binds));
	}

	std::unique_ptr<Model::Node> Model::ParseNode(const aiNode& node) noexcept(!IS_DEBUG)
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

	Model::Model(Graphics& gfx, const std::string& file, const DirectX::XMFLOAT3& position, const std::string& modelName, float scale)
		: name(modelName)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(file, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices |
			aiProcess_ConvertToLeftHanded | aiProcess_GenSmoothNormals | aiProcess_GenUVCoords | aiProcess_CalcTangentSpace);
		if (!scene)
			throw ModelException(__LINE__, __FILE__, importer.GetErrorString());

		meshes.reserve(scene->mNumMeshes);
		std::string path = std::filesystem::path(file).remove_filename().string();
		for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
			meshes.emplace_back(ParseMesh(gfx, path, *scene->mMeshes[i], scene->mMaterials));
		root = ParseNode(*scene->mRootNode);
		root->SetScale(scale);
		root->SetPos(position);
		window = std::make_unique<Window>(const_cast<Model*>(this));
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