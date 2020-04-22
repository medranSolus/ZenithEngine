#include "SolidGlobe.h"
#include "Primitives.h"
#include "GfxResources.h"
#include "Math.h"

namespace GFX::Shape
{
	SolidGlobe::SolidGlobe(Graphics& gfx, const DirectX::XMFLOAT3& position, const std::string& name, Data::ColorFloat4 material,
		unsigned int latitudeDensity, unsigned int longitudeDensity, float width, float height, float length)
		: BaseShape(gfx, *this), Object(position, name), sizes(width, height, length)
	{
		auto vertexShader = Resource::VertexShader::Get(gfx, "SolidVS.cso");
		auto bytecodeVS = vertexShader->GetBytecode();
		AddBind(vertexShader);
		AddBind(Resource::PixelShader::Get(gfx, "SolidPS.cso"));

		std::string typeName = Primitive::Sphere::GetNameUVSolid(latitudeDensity, longitudeDensity);
		if (Resource::VertexBuffer::NotStored(typeName) && Resource::IndexBuffer::NotStored(typeName))
		{
			auto list = Primitive::Sphere::MakeSolidUV(latitudeDensity, longitudeDensity);
			AddBind(Resource::VertexBuffer::Get(gfx, typeName, list.vertices));
			AddBind(Resource::IndexBuffer::Get(gfx, typeName, list.indices));
			AddBind(Resource::InputLayout::Get(gfx, list.vertices.GetLayout(), bytecodeVS));
		}
		else
		{
			Primitive::IndexedTriangleList list;
			AddBind(Resource::VertexBuffer::Get(gfx, typeName, list.vertices));
			AddBind(Resource::IndexBuffer::Get(gfx, typeName, list.indices));
			AddBind(Resource::InputLayout::Get(gfx, Primitive::Sphere::GetLayoutUVSolid(), bytecodeVS));
		}

		Data::CBuffer::Solid buffer{ material };
		auto materialData = Resource::ConstBufferPixel<Data::CBuffer::Solid>::Get(gfx, name, buffer);
		materialBuffer = materialData.get();
		AddBind(std::move(materialData));
		
		UpdateTransformMatrix();
	}

	void SolidGlobe::ShowWindow(Graphics& gfx) noexcept
	{
		Object::ShowWindow(gfx);
		BaseShape::ShowWindow(gfx);
	}

	void SolidGlobe::UpdateTransformMatrix() noexcept
	{
		DirectX::XMStoreFloat4x4(transform.get(),
			DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&sizes)) *
			CreateTransformMatrix());
	}
}