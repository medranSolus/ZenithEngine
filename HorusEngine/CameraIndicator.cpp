#include "CameraIndicator.h"
#include "Primitives.h"
#include "Visuals.h"

namespace GFX::Shape
{
	CameraIndicator::CameraIndicator(Graphics& gfx, Pipeline::RenderGraph& graph, const DirectX::XMFLOAT3& position, Data::ColorFloat3 color)
		: BaseShape(gfx), Object(position)
	{
		const std::string typeName = typeid(CameraIndicator).name();
		if (Resource::VertexBuffer::NotStored(typeName) && Resource::IndexBuffer::NotStored(typeName))
		{
			Data::VertexBufferData vertices(std::make_shared<Data::VertexLayout>(), 6U);
			vertices[0].SetByIndex(0, position);
			vertices[1].SetByIndex(0, std::move(DirectX::XMFLOAT3(-1.0f, 0.7f, 2.0f)));
			vertices[2].SetByIndex(0, std::move(DirectX::XMFLOAT3(1.0f, 0.7f, 2.0f)));
			vertices[3].SetByIndex(0, std::move(DirectX::XMFLOAT3(1.0f, -0.7f, 2.0f)));
			vertices[4].SetByIndex(0, std::move(DirectX::XMFLOAT3(-1.0f, -0.7f, 2.0f)));
			vertices[5].SetByIndex(0, std::move(DirectX::XMFLOAT3(0.0f, 0.7f, 2.0f)));
			SetVertexBuffer(Resource::VertexBuffer::Get(gfx, typeName, vertices));

			std::vector<unsigned int> indices =
			{
				0,4, 0,1, 0,5, 0,2, 0,3,
				4,1, 1,2, 2,3, 3,4
			};
			SetIndexBuffer(Resource::IndexBuffer::Get(gfx, typeName, indices));
		}
		else
		{
			Primitive::IndexedTriangleList list;
			SetVertexBuffer(Resource::VertexBuffer::Get(gfx, typeName, list.vertices));
			SetIndexBuffer(Resource::IndexBuffer::Get(gfx, typeName, list.indices));
		}
		SetTopology(gfx, D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_LINELIST);

		std::vector<std::shared_ptr<Pipeline::Technique>> techniques;
		auto material = std::make_shared<Visual::Material>(gfx, color, name);
		auto vertexLayout = material->GerVertexLayout();

		techniques.emplace_back(std::make_shared<Pipeline::Technique>("Solid"));
		techniques.back()->AddStep({ graph, "lambertian", std::move(material) });
		SetTechniques(gfx, std::move(techniques), *this);
	}

	void CameraIndicator::Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept
	{
		Object::Accept(gfx, probe);
		BaseShape::Accept(gfx, probe);
	}
}