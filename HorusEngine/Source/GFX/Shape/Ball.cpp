#include "GFX/Shape/Ball.h"
#include "GFX/Primitive/Sphere.h"
#include "GFX/Pipeline/TechniqueFactory.h"

namespace GFX::Shape
{
	Ball::Ball(Graphics& gfx, Pipeline::RenderGraph& graph, const Float3& position,
		std::string&& name, const ColorF4& color, U32 density, float radius)
		: IShape(gfx, position, std::forward<std::string>(name), radius)
	{
		std::string typeName = Primitive::Sphere::GetNameIco(density);
		if (Resource::VertexBuffer::NotStored(typeName) && Resource::IndexBuffer::NotStored(typeName))
		{
			auto list = Primitive::Sphere::MakeIco(density);
			SetVertexBuffer(Resource::VertexBuffer::Get(gfx, typeName, std::move(list.Vertices)));
			SetIndexBuffer(Resource::IndexBuffer::Get(gfx, typeName, std::move(list.Indices)));
		}
		else
		{
			SetVertexBuffer(Resource::VertexBuffer::Get(gfx, typeName, {}));
			SetIndexBuffer(Resource::IndexBuffer::Get(gfx, typeName, {}));
		}

		std::vector<Pipeline::Technique> techniques;
		techniques.reserve(3);
		auto material = std::make_shared<Visual::Material>(gfx, color, GetName());
		auto vertexLayout = material->GerVertexLayout();

		techniques.emplace_back(Pipeline::TechniqueFactory::MakeShadowMap(gfx, graph, material));
		techniques.emplace_back(Pipeline::TechniqueFactory::MakeLambertian(gfx, graph, std::move(material)));
		techniques.emplace_back(Pipeline::TechniqueFactory::MakeOutlineBlur(gfx, graph, GetName(), std::move(vertexLayout)));
		SetTechniques(gfx, std::move(techniques), *this);
	}
}