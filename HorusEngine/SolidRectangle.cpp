#include "SolidRectangle.h"
#include "Primitives.h"
#include "TechniqueFactory.h"

namespace GFX::Shape
{
	SolidRectangle::SolidRectangle(Graphics& gfx, Pipeline::RenderGraph& graph, const DirectX::XMFLOAT3& position,
		const std::string& name, Data::ColorFloat3 color, float width, float height)
		: IShape(gfx, position, name), width(width), height(height)
	{
		std::string typeName = Primitive::Square::GetName();
		if (Resource::VertexBuffer::NotStored(typeName) && Resource::IndexBuffer::NotStored(typeName))
		{
			auto list = Primitive::Square::Make();
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
		techniques.reserve(3);
		auto material = std::make_shared<Visual::Material>(gfx, color, name);
		auto vertexLayout = material->GerVertexLayout();

		techniques.emplace_back(Pipeline::TechniqueFactory::MakeShadowMap(gfx, graph, vertexLayout));
		techniques.emplace_back(Pipeline::TechniqueFactory::MakeLambertian(gfx, graph, std::move(material)));
		techniques.emplace_back(Pipeline::TechniqueFactory::MakeOutlineScale(gfx, graph, name, std::move(vertexLayout)));
		SetTechniques(gfx, std::move(techniques), *this);

		UpdateTransformMatrix();
	}

	void SolidRectangle::UpdateTransformMatrix() noexcept
	{
		DirectX::XMStoreFloat4x4(transform.get(),
			DirectX::XMMatrixScaling(width, height, 1.0f) *
			CreateTransformMatrix());
	}
}