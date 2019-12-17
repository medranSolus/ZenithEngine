#include "Model.h"
#include "GfxResources.h"
#include "VertexDataBuffer.h"
#include "Primitives.h"
#include "Math.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

namespace GFX::Object
{
	Model::Model(Graphics & gfx, const std::string & filename, float x0, float y0, float z0, float scale)
		: ObjectBase(x0, y0, z0)
	{
		if (!IsStaticInit())
		{
			Assimp::Importer importer;
			const aiScene * model = importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_ConvertToLeftHanded);
			const aiMesh * mesh = model->mMeshes[0];

			BasicType::VertexDataBuffer buffer(std::move(BasicType::VertexLayout{}
				.Append(VertexAttribute::Position3D)
				.Append(VertexAttribute::Normal)));

			for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
				buffer.EmplaceBack(std::move(DirectX::XMFLOAT3(mesh->mVertices[i].x * scale, mesh->mVertices[i].y * scale, mesh->mVertices[i].z * scale)),
					*reinterpret_cast<DirectX::XMFLOAT3*>(&mesh->mNormals[i]));
			AddStaticBind(std::make_unique<Resource::VertexBuffer>(gfx, buffer));
			std::vector<unsigned int> indices;
			indices.reserve(mesh->mNumFaces * 3);
			for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
			{
				const aiFace & face = mesh->mFaces[i];
				assert(face.mNumIndices == 3);
				indices.emplace_back(face.mIndices[0]);
				indices.emplace_back(face.mIndices[1]);
				indices.emplace_back(face.mIndices[2]);
			}
			AddStaticIndexBuffer(std::make_unique<Resource::IndexBuffer>(gfx, std::move(indices)));

			auto vertexShader = std::make_unique<Resource::VertexShader>(gfx, L"PhongVS.cso");
			auto bytecodeVS = vertexShader->GetBytecode();
			AddStaticBind(std::move(vertexShader));
			AddStaticBind(std::make_unique<Resource::PixelShader>(gfx, L"PhongPS.cso"));

			AddStaticBind(std::make_unique<Resource::InputLayout>(gfx, buffer.GetLayout().GetDXLayout(), bytecodeVS));

			AddStaticBind(std::make_unique<Resource::Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST)); // Mesh: D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ
		}
		AddBind(std::make_unique<Resource::ConstantTransformBuffer>(gfx, *this));

		std::mt19937_64 engine(std::random_device{}());
		Resource::ObjectConstantBuffer buffer;
		buffer.materialColor = randColor(engine);
		buffer.specularIntensity = 0.6f;
		buffer.specularPower = 60.0f;
		AddBind(std::make_unique<Resource::ConstantPixelBuffer<Resource::ObjectConstantBuffer>>(gfx, buffer, 1U));
	}

	void Model::Update(float dX, float dY, float dZ, float angleDZ, float angleDX, float angleDY) noexcept
	{
		DirectX::XMStoreFloat3(&pos, DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&pos), DirectX::XMVectorSet(dX, dY, dZ, 0.0f)));
		DirectX::XMStoreFloat3(&angle,
			DirectX::XMVectorModAngles(DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&angle),
				DirectX::XMVectorSet(angleDX, angleDY, angleDZ, 0.0f))));
	}

	DirectX::XMMATRIX Model::GetTransformMatrix() const noexcept
	{
		return DirectX::XMMatrixRotationRollPitchYawFromVector(DirectX::XMLoadFloat3(&angle)) *
			DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&pos));
	}
}
