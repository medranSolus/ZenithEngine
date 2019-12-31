#include "Triangle.h"
#include "Primitives.h"
#include "GfxResources.h"
#include "Math.h"

namespace GFX::Object
{
	Triangle::Triangle(Graphics & gfx, float x0, float y0, float z0, float down, float left, float right) : pos(x0, y0, z0)
	{
		const float leftPow2 = left * left;
		const float vertex3X = (right * right - leftPow2 - down * down) / (-2.0f * down);
		const float centerX = (vertex3X + down) / 3;
		const float centerY = sqrtf(leftPow2 - vertex3X * vertex3X) / 3;
		std::mt19937_64 engine(std::random_device{}());

		BasicType::VertexDataBuffer vertices(std::move(BasicType::VertexLayout{}
			.Append(VertexAttribute::Position3D)
			.Append(VertexAttribute::ColorFloat)));

		vertices.EmplaceBack(DirectX::XMFLOAT3(-centerX, -centerY, 0.0f), randColor(engine));
		vertices.EmplaceBack(DirectX::XMFLOAT3(vertex3X - centerX, 2.0f * centerY, 0.0f), randColor(engine));
		vertices.EmplaceBack(DirectX::XMFLOAT3(down - centerX, -centerY, 0.0f), randColor(engine));

		AddBind(std::make_unique<Resource::VertexBuffer>(gfx, vertices));

		if (!IsStaticInit())
		{
			auto vertexShader = std::make_unique<Resource::VertexShader>(gfx, L"ColorBlendVS.cso");
			auto bytecodeVS = vertexShader->GetBytecode();
			AddStaticBind(std::move(vertexShader));

			AddStaticBind(std::make_unique<Resource::PixelShader>(gfx, L"ColorBlendPS.cso"));

			AddStaticIndexBuffer(std::make_unique<Resource::IndexBuffer>(gfx, std::move(std::vector<unsigned int>({ 0, 1, 2 }))));

			AddStaticBind(std::make_unique<Resource::InputLayout>(gfx, vertices.GetLayout().GetDXLayout(), bytecodeVS));

			AddStaticBind(std::make_unique<Resource::Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
		}
		AddBind(std::make_unique<Resource::ConstantTransformBuffer>(gfx, *this));
	}

	void Triangle::Update(float dX, float dY, float dZ, float angleDZ, float angleDX, float angleDY) noexcept
	{
		DirectX::XMStoreFloat3(&pos, DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&pos), DirectX::XMVectorSet(dX, dY, dZ, 0.0f)));
		DirectX::XMStoreFloat3(&angle,
			DirectX::XMVectorModAngles(DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&angle),
				DirectX::XMVectorSet(angleDX, angleDY, angleDZ, 0.0f))));
	}

	DirectX::XMMATRIX Triangle::GetTransformMatrix() const noexcept
	{
		return DirectX::XMMatrixRotationRollPitchYawFromVector(DirectX::XMLoadFloat3(&angle)) *
			DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&pos));
	}
}
