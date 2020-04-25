#include "Ball.h"
#include "Primitives.h"
#include "GfxResources.h"
#include "Math.h"

namespace GFX::Shape
{
	Ball::Ball(Graphics& gfx, const DirectX::XMFLOAT3& position, const std::string& name, Data::ColorFloat4 material, unsigned int density, float radius)
		: BaseShape(gfx, *this), Object(position, name, radius)
	{
		/*auto vertexShader = Resource::VertexShader::Get(gfx, "PhongVS.cso");
		auto bytecodeVS = vertexShader->GetBytecode();
		AddBind(vertexShader);
		AddBind(Resource::PixelShader::Get(gfx, "PhongPS.cso"));

		std::string typeName = Primitive::Sphere::GetNameIco(density);
		if (Resource::VertexBuffer::NotStored(typeName) && Resource::IndexBuffer::NotStored(typeName))
		{
			auto list = Primitive::Sphere::MakeIco(density);
			AddBind(Resource::VertexBuffer::Get(gfx, typeName, list.vertices));
			AddBind(Resource::IndexBuffer::Get(gfx, typeName, list.indices));
			AddBind(Resource::InputLayout::Get(gfx, list.vertices.GetLayout(), bytecodeVS));
		}
		else
		{
			Primitive::IndexedTriangleList list;
			AddBind(Resource::VertexBuffer::Get(gfx, typeName, list.vertices));
			AddBind(Resource::IndexBuffer::Get(gfx, typeName, list.indices));
			AddBind(Resource::InputLayout::Get(gfx, Primitive::Sphere::GetLayoutIco(), bytecodeVS));
		}

		Data::CBuffer::Phong buffer;
		buffer.materialColor = material;
		buffer.specularIntensity = 0.6f;
		buffer.specularPower = 60.0f;
		AddBind(Resource::ConstBufferPixel<Data::CBuffer::Phong>::Get(gfx, name, buffer, 1U));*/
	}

	void Ball::ShowWindow(Graphics& gfx) noexcept
	{
		Object::ShowWindow(gfx);
		BaseShape::ShowWindow(gfx);
	}
}