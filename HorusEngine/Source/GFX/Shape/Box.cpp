#include "GFX/Shape/Box.h"
#include "GFX/Primitive/Cube.h"
#include "GFX/Pipeline/TechniqueFactory.h"

namespace GFX::Shape
{
	Box::Box(Graphics& gfx, Pipeline::RenderGraph& graph, const Float3& position,
		std::string&& name, const ColorF4& color, float width, float height, float length)
		: IShape(gfx, position, std::forward<std::string>(name)), sizes(width, height, length)
	{
		std::string typeName = Primitive::Cube::GetName();
		if (Resource::VertexBuffer::NotStored(typeName) && Resource::IndexBuffer::NotStored(typeName))
		{
			auto list = Primitive::Cube::Make();
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
		techniques.emplace_back(Pipeline::TechniqueFactory::MakeOutlineScale(gfx, graph, GetName(), std::move(vertexLayout)));
		SetTechniques(gfx, std::move(techniques), *this);

		UpdateTransformMatrix();
	}

	void Box::UpdateTransformMatrix() noexcept
	{
		Math::XMStoreFloat4x4(transform.get(),
			Math::XMMatrixScalingFromVector(Math::XMLoadFloat3(&sizes)) *
			CreateTransformMatrix());
	}
}