#include "Rectangle.h"
#include "Primitives.h"
#include "GfxResources.h"
#include "App.h"
#include "Math.h"

inline float randomVertex()
{
	return (rand() % 200 - 100) / 100.0f;
}

namespace GFX::Object
{
	Rectangle::Rectangle(Graphics & gfx, float x0, float y0, float z0, float width, float height, bool isRandom)
		: ObjectBase(x0, y0, z0), width(width), height(height)
	{
		auto list = Primitive::Square::Make<Primitive::VertexColor>();
		if (isRandom)
		{
			for (unsigned char i = 0; i < 4; ++i)
			{
				list.vertices.at(i).pos.x += randomVertex();
				list.vertices.at(i).pos.y += randomVertex();
			}
		}
		for (unsigned char i = 0; i < 4; ++i)
			list.vertices.at(i).col = randColor();
		AddBind(std::make_unique<Resource::VertexBuffer>(gfx, list.vertices));

		if (!IsStaticInit())
		{
			auto vertexShader = std::make_unique<Resource::VertexShader>(gfx, L"ColorBlendVS.cso");
			auto bytecodeVS = vertexShader->GetBytecode();
			AddStaticBind(std::move(vertexShader));

			AddStaticBind(std::make_unique<Resource::PixelShader>(gfx, L"ColorBlendPS.cso"));

			AddStaticIndexBuffer(std::make_unique<Resource::IndexBuffer>(gfx, list.indices));

			const std::vector<D3D11_INPUT_ELEMENT_DESC> inputDesc =
			{
				{ "Position", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "Color", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0 }
			};
			AddStaticBind(std::make_unique<Resource::InputLayout>(gfx, inputDesc, bytecodeVS));

			AddStaticBind(std::make_unique<Resource::Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
		}
		AddBind(std::make_unique<Resource::ConstantTransformBuffer>(gfx, *this));
	}

	void Rectangle::Update(float dX, float dY, float dZ, float angleDZ, float angleDX, float angleDY) noexcept
	{
		DirectX::XMStoreFloat3(&pos, DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&pos), DirectX::XMVectorSet(dX, dY, dZ, 0.0f)));
		DirectX::XMStoreFloat3(&angle,
			DirectX::XMVectorModAngles(DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&angle),
				DirectX::XMVectorSet(angleDX, angleDY, angleDZ, 0.0f))));
	}

	DirectX::XMMATRIX Rectangle::GetTransformMatrix() const noexcept
	{
		return DirectX::XMMatrixScaling(width, height, 1.0f) *
			DirectX::XMMatrixRotationRollPitchYawFromVector(DirectX::XMLoadFloat3(&angle)) *
			DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&pos));
	}
}
