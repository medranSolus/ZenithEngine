#include "SolidRectangle.h"
#include "Primitives.h"
#include "GfxResources.h"
#include "Visuals.h"

namespace GFX::Shape
{
	SolidRectangle::SolidRectangle(Graphics& gfx, const DirectX::XMFLOAT3& position, const std::string& name, Data::ColorFloat3 color, float width, float height)
		: BaseShape(gfx), Object(position, name), width(width), height(height)
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
		auto material = std::make_shared<Visual::Material>(gfx, color, name);
		auto vertexLayout = material->GerVertexLayout();

		techniques.emplace_back(std::make_shared<Pipeline::Technique>("Solid"));
		techniques.back()->AddStep({ 0, std::move(material) });

		techniques.emplace_back(std::make_shared<Pipeline::Technique>("Outline", false));
		techniques.back()->AddStep({ 1, std::make_shared<Visual::OutlineWrite>(gfx, vertexLayout) });
		techniques.back()->AddStep({ 2, std::make_shared<Visual::OutlineMask>(gfx, name + "Outline", std::move(Data::ColorFloat3(1.0f, 1.0f, 0.0f)), std::move(vertexLayout)) });
		SetTechniques(gfx, std::move(techniques), *this);

		UpdateTransformMatrix();
	}

	void SolidRectangle::Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept
	{
		Object::Accept(gfx, probe);
		BaseShape::Accept(gfx, probe);
	}

	void SolidRectangle::UpdateTransformMatrix() noexcept
	{
		DirectX::XMStoreFloat4x4(transform.get(),
			DirectX::XMMatrixScaling(width, height, 1.0f) *
			CreateTransformMatrix());
	}
}