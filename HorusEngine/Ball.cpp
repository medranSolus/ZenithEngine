#include "Ball.h"
#include "Primitives.h"
#include "TechniqueFactory.h"

namespace GFX::Shape
{
	Ball::Ball(Graphics& gfx, Pipeline::RenderGraph& graph, const DirectX::XMFLOAT3& position, const std::string& name, Data::ColorFloat4 color, unsigned int density, float radius)
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
		techniques.reserve(2);
		auto material = std::make_shared<Visual::Material>(gfx, color, name);
		auto vertexLayout = material->GerVertexLayout();

		techniques.emplace_back(Pipeline::TechniqueFactory::MakeLambertian(graph, RenderChannel::Main, std::move(material)));
		techniques.emplace_back(Pipeline::TechniqueFactory::MakeOutlineScale(gfx, graph, RenderChannel::Main, name, std::move(vertexLayout)));
		SetTechniques(gfx, std::move(techniques), *this);
	}

	void Ball::Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept
	{
		Object::Accept(gfx, probe);
		BaseShape::Accept(gfx, probe);
	}
}