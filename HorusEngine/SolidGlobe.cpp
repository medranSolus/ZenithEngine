#include "SolidGlobe.h"
#include "Primitives.h"
#include "Material.h"

namespace GFX::Shape
{
	SolidGlobe::SolidGlobe(Graphics& gfx, const DirectX::XMFLOAT3& position, const std::string& name, Data::ColorFloat4 color,
		unsigned int latitudeDensity, unsigned int longitudeDensity, float width, float height, float length)
		: BaseShape(gfx, *this), Object(position, name), sizes(width, height, length)
	{
		std::string typeName = Primitive::Sphere::GetNameUVSolid(latitudeDensity, longitudeDensity);
		if (Resource::VertexBuffer::NotStored(typeName) && Resource::IndexBuffer::NotStored(typeName))
		{
			auto list = Primitive::Sphere::MakeSolidUV(latitudeDensity, longitudeDensity);
			SetVertexBuffer(Resource::VertexBuffer::Get(gfx, typeName, list.vertices));
			SetIndexBuffer(Resource::IndexBuffer::Get(gfx, typeName, list.indices));
		}
		else
		{
			Primitive::IndexedTriangleList list;
			SetVertexBuffer(Resource::VertexBuffer::Get(gfx, typeName, list.vertices));
			SetIndexBuffer(Resource::IndexBuffer::Get(gfx, typeName, list.indices));
		}

		std::vector<std::shared_ptr<Pipeline::Technique>> techniques;
		techniques.emplace_back(std::make_shared<Pipeline::Technique>("Solid"));
		auto material = std::make_shared<Visual::Material>(gfx, color, name);
		materialBuffer = &material->GetPixelBuffer();
		techniques.back()->AddStep({ 0, std::move(material) });
		SetTechniques(std::move(techniques));

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