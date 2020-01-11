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
		AddBind(std::make_shared<Resource::VertexBuffer>(gfx, list.vertices));


		auto vertexShader = std::make_shared<Resource::VertexShader>(gfx, L"ColorBlendVS.cso");
		auto bytecodeVS = vertexShader->GetBytecode();
		AddBind(vertexShader);

		AddBind(std::make_shared<Resource::PixelShader>(gfx, L"ColorBlendPS.cso"));

		AddBind(std::make_shared<Resource::IndexBuffer>(gfx, list.indices));

		AddBind(std::make_shared<Resource::InputLayout>(gfx, list.vertices.GetLayout().GetDXLayout(), bytecodeVS));

		AddBind(std::make_shared<Resource::Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));


		AddBind(std::make_shared<Resource::ConstantTransformBuffer>(gfx, *this));

		UpdateScalingMatrix();
	}

	void Rectangle::UpdateScalingMatrix() noexcept
	{
		DirectX::XMStoreFloat4x4(scaling.get(),
			DirectX::XMMatrixScaling(width, height, 1.0f) *
			DirectX::XMMatrixScaling(scale, scale, scale));
	}
}
