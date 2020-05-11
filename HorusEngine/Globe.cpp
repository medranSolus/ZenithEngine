#include "Globe.h"
#include "Primitives.h"
#include "GfxResources.h"
#include "Visuals.h"

namespace GFX::Shape
{
	Globe::Globe(Graphics& gfx, const DirectX::XMFLOAT3& position, const std::string& name, Data::ColorFloat4 color,
		unsigned int latitudeDensity, unsigned int longitudeDensity, float width, float height, float length)
		: BaseShape(gfx), Object(position, name), sizes(width, height, length)
	{
		std::string typeName = Primitive::Sphere::GetNameUV(latitudeDensity, longitudeDensity);
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
		auto material = std::make_shared<Visual::Material>(gfx, color, name);
		auto vertexLayout = material->GerVertexLayout();

		techniques.emplace_back(std::make_shared<Pipeline::Technique>("Phong"));
		techniques.back()->AddStep({ 0, std::move(material) });

		techniques.emplace_back(std::make_shared<Pipeline::Technique>("Outline", false));
		techniques.back()->AddStep({ 1, std::make_shared<Visual::OutlineWrite>(gfx, vertexLayout) });
		techniques.back()->AddStep({ 2, std::make_shared<Visual::OutlineMask>(gfx, name + "Outline", std::move(Data::ColorFloat3(1.0f, 1.0f, 0.0f)), std::move(vertexLayout)) });
		SetTechniques(gfx, std::move(techniques), *this);

		UpdateTransformMatrix();
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