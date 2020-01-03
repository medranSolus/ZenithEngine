#include "Rectangle.h"
#include "Primitives.h"
#include "GfxResources.h"
#include "Math.h"

inline float randomVertex()
{
	return (rand() % 200 - 100) / 100.0f;
}

namespace GFX::Shape
{
	Rectangle::Rectangle(Graphics & gfx, const DirectX::XMFLOAT3 & position, const std::string & name, float width, float height, bool isRandom)
		: Object(position, name), width(width), height(height)
	{
		auto list = Primitive::Square::Make({ VertexAttribute::ColorFloat });
		if (isRandom)
		{
			for (unsigned char i = 0; i < 4; ++i)
			{
				list.vertices[i].Get<VertexAttribute::Position3D>().x += randomVertex();
				list.vertices[i].Get<VertexAttribute::Position3D>().y += randomVertex();
			}
		}
		std::mt19937_64 engine(std::random_device{}());
		for (unsigned char i = 0; i < 4; ++i)
			list.vertices[i].Get<VertexAttribute::ColorFloat>() = std::move(randColor(engine));
		AddBind(std::make_unique<Resource::VertexBuffer>(gfx, list.vertices));

		if (!IsStaticInit())
		{
			auto vertexShader = std::make_unique<Resource::VertexShader>(gfx, L"ColorBlendVS.cso");
			auto bytecodeVS = vertexShader->GetBytecode();
			AddStaticBind(std::move(vertexShader));

			AddStaticBind(std::make_unique<Resource::PixelShader>(gfx, L"ColorBlendPS.cso"));

			AddStaticIndexBuffer(std::make_unique<Resource::IndexBuffer>(gfx, list.indices));

			AddStaticBind(std::make_unique<Resource::InputLayout>(gfx, list.vertices.GetLayout().GetDXLayout(), bytecodeVS));

			AddStaticBind(std::make_unique<Resource::Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
		}
		AddBind(std::make_unique<Resource::ConstantTransformBuffer>(gfx, *this));
	}
	
	DirectX::XMMATRIX Rectangle::GetTransformMatrix() const noexcept
	{
		return DirectX::XMMatrixScaling(width, height, 1.0f) *
			DirectX::XMMatrixRotationRollPitchYawFromVector(DirectX::XMLoadFloat3(&angle)) *
			DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&pos));
	}
}
