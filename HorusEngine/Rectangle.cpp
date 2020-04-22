#include "Rectangle.h"
#include "Primitives.h"
#include "GfxResources.h"
#include "Math.h"

namespace GFX::Shape
{
	Rectangle::Rectangle(Graphics& gfx, const DirectX::XMFLOAT3& position, const std::string& name, float width, float height)
		: BaseShape(gfx, *this), Object(position, name), width(width), height(height)
	{
		auto list = Primitive::Square::Make({ VertexAttribute::ColorFloat4 });
		std::mt19937_64 engine(std::random_device{}());
		for (unsigned char i = 0; i < 4; ++i)
			list.vertices[i].Get<VertexAttribute::ColorFloat4>() = std::move(randColor(engine));
		AddBind(Resource::VertexBuffer::Get(gfx, name, list.vertices));
		AddBind(Resource::IndexBuffer::Get(gfx, name, list.indices));

		auto vertexShader = Resource::VertexShader::Get(gfx, "ColorBlendVS.cso");
		auto bytecodeVS = vertexShader->GetBytecode();
		AddBind(vertexShader);
		AddBind(Resource::PixelShader::Get(gfx, "ColorBlendPS.cso"));
		AddBind(Resource::InputLayout::Get(gfx, list.vertices.GetLayout(), bytecodeVS));

		UpdateTransformMatrix();
	}

	void Rectangle::ShowWindow(Graphics& gfx) noexcept
	{
		Object::ShowWindow(gfx);
		BaseShape::ShowWindow(gfx);
	}

	void Rectangle::UpdateTransformMatrix() noexcept
	{
		DirectX::XMStoreFloat4x4(transform.get(),
			DirectX::XMMatrixScaling(width, height, 1.0f) *
			CreateTransformMatrix());
	}
}