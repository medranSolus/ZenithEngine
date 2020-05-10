#include "Globe.h"
#include "Primitives.h"
#include "GfxResources.h"
#include "Math.h"

namespace GFX::Shape
{
	Globe::Globe(Graphics& gfx, const DirectX::XMFLOAT3& position, const std::string& name, Data::ColorFloat4 material,
		unsigned int latitudeDensity, unsigned int longitudeDensity, float width, float height, float length)
		: BaseShape(gfx, *this), Object(position, name), sizes(width, height, length)
	{
		/*auto vertexShader = Resource::VertexShader::Get(gfx, "PhongVS.cso");
		auto bytecodeVS = vertexShader->GetBytecode();
		AddBind(vertexShader);
		AddBind(Resource::PixelShader::Get(gfx, "PhongPS.cso"));

		std::string typeName = Primitive::Sphere::GetNameUV(latitudeDensity, longitudeDensity);
		if (Resource::VertexBuffer::NotStored(typeName) && Resource::IndexBuffer::NotStored(typeName))
		{
			auto list = Primitive::Sphere::MakeUV(latitudeDensity, longitudeDensity);
			AddBind(Resource::VertexBuffer::Get(gfx, typeName, list.vertices));
			AddBind(Resource::IndexBuffer::Get(gfx, typeName, list.indices));
			AddBind(Resource::InputLayout::Get(gfx, list.vertices.GetLayout(), bytecodeVS));
		}
		else
		{
			Primitive::IndexedTriangleList list;
			AddBind(Resource::VertexBuffer::Get(gfx, typeName, list.vertices));
			AddBind(Resource::IndexBuffer::Get(gfx, typeName, list.indices));
			AddBind(Resource::InputLayout::Get(gfx, Primitive::Sphere::GetLayoutUV(), bytecodeVS));
		}

		Data::CBuffer::Phong buffer;
		buffer.materialColor = material;
		buffer.specularIntensity = 0.6f;
		buffer.specularPower = 60.0f;
		AddBind(Resource::ConstBufferPixel<Data::CBuffer::Phong>::Get(gfx, name, buffer, 1U));

		UpdateTransformMatrix();*/
	}

	void Globe::Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept
	{
		Object::Accept(gfx, probe);
		BaseShape::Accept(gfx, probe);
	}

	void Globe::UpdateTransformMatrix() noexcept
	{
		DirectX::XMStoreFloat4x4(transform.get(),
			DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&sizes)) *
			CreateTransformMatrix());
	}
}