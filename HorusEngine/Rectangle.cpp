#include "Rectangle.h"
#include "Primitives.h"
#include "GfxResources.h"
#include "Math.h"

namespace GFX::Shape
{
	Rectangle::Rectangle(Graphics & gfx, const DirectX::XMFLOAT3 & position, const std::string & name, float width, float height)
		: Object(position, name), width(width), height(height)
	{
		auto list = Primitive::Square::Make({ VertexAttribute::ColorFloat });
		std::mt19937_64 engine(std::random_device{}());
		for (unsigned char i = 0; i < 4; ++i)
			list.vertices[i].Get<VertexAttribute::ColorFloat>() = std::move(randColor(engine));
		AddBind(Resource::VertexBuffer::Get(gfx, typeid(Rectangle).name() + name, list.vertices));


		auto vertexShader = Resource::VertexShader::Get(gfx, "ColorBlendVS.cso");
		auto bytecodeVS = vertexShader->GetBytecode();
		AddBind(vertexShader);

		AddBind(Resource::PixelShader::Get(gfx, "ColorBlendPS.cso"));

		AddBind(Resource::IndexBuffer::Get(gfx, typeid(Rectangle).name() + name, list.indices));

		AddBind(Resource::InputLayout::Get(gfx, list.vertices.GetLayout(), bytecodeVS));

		AddBind(Resource::Topology::Get(gfx, D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));

		AddBind(Resource::ConstantTransformBuffer::Get(gfx, *this));

		UpdateScalingMatrix();
	}

	void Rectangle::UpdateScalingMatrix() noexcept
	{
		DirectX::XMStoreFloat4x4(scaling.get(),
			DirectX::XMMatrixScaling(width, height, 1.0f) *
			DirectX::XMMatrixScaling(scale, scale, scale));
	}
}
