#include "Ball.h"
#include "Primitives.h"
#include "TechniqueFactory.h"

namespace GFX::Shape
{
	Ball::Ball(Graphics& gfx, Pipeline::RenderGraph& graph, const DirectX::XMFLOAT3& position,
		const std::string& name, Data::ColorFloat4 color, unsigned int density, float radius)
		: IShape(gfx, position, name, radius)
	{
		const std::string typeName = Primitive::Sphere::GetNameIco(density);
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

		std::vector<Pipeline::Technique> techniques;
		techniques.reserve(3);
		auto material = std::make_shared<Visual::Material>(gfx, color, name);
		auto vertexLayout = material->GerVertexLayout();

		techniques.emplace_back(Pipeline::TechniqueFactory::MakeShadowMap(gfx, graph, material));
		techniques.emplace_back(Pipeline::TechniqueFactory::MakeLambertian(gfx, graph, std::move(material)));
		techniques.emplace_back(Pipeline::TechniqueFactory::MakeOutlineBlur(gfx, graph, name, std::move(vertexLayout)));
		SetTechniques(gfx, std::move(techniques), *this);
	}
}