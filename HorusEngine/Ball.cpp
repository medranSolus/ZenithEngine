#include "Ball.h"
#include "Primitives.h"
#include "GfxResources.h"
#include "Visuals.h"

namespace GFX::Shape
{
	Ball::Ball(Graphics& gfx, const DirectX::XMFLOAT3& position, const std::string& name, Data::ColorFloat4 color, unsigned int density, float radius)
		: BaseShape(gfx), Object(position, name, radius)
	{
		std::string typeName = Primitive::Sphere::GetNameIco(density);
		if (Resource::VertexBuffer::NotStored(typeName) && Resource::IndexBuffer::NotStored(typeName))
		{
			auto list = Primitive::Sphere::MakeIco(density);
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
		techniques.back()->AddStep({ 2, std::make_shared<Visual::OutlineMaskScale>(gfx, name + "Outline", std::move(Data::ColorFloat3(1.0f, 1.0f, 0.0f)), std::move(vertexLayout)) });
		SetTechniques(gfx, std::move(techniques), *this);
	}

	void Ball::Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept
	{
		Object::Accept(gfx, probe);
		BaseShape::Accept(gfx, probe);
	}
}