#include "CameraIndicator.h"
#include "Primitives.h"
#include "TechniqueFactory.h"

namespace GFX::Shape
{
	CameraIndicator::CameraIndicator(Graphics& gfx, Pipeline::RenderGraph& graph, const DirectX::XMFLOAT3& position,
		const std::string& name, Data::ColorFloat3 color)
		: BaseShape(gfx), Object(position, name)
	{
		const std::string typeName = typeid(CameraIndicator).name();
		if (Resource::VertexBuffer::NotStored(typeName) && Resource::IndexBuffer::NotStored(typeName))
		{
			Data::VertexBufferData vertices(std::make_shared<Data::VertexLayout>(), 8U);
			constexpr float length = 1.0f, width = 1.0f, height = 0.7f;

			vertices[0].SetByIndex(0, std::move(DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f)));
			vertices[1].SetByIndex(0, std::move(DirectX::XMFLOAT3(-width, height, length)));
			vertices[2].SetByIndex(0, std::move(DirectX::XMFLOAT3(width, height, length)));
			vertices[3].SetByIndex(0, std::move(DirectX::XMFLOAT3(width, -height, length)));
			vertices[4].SetByIndex(0, std::move(DirectX::XMFLOAT3(-width, -height, length)));
			vertices[5].SetByIndex(0, std::move(DirectX::XMFLOAT3(width / 2.0f, height + 0.15f, length)));
			vertices[6].SetByIndex(0, std::move(DirectX::XMFLOAT3(0.0f, height * 2.0f, length)));
			vertices[7].SetByIndex(0, std::move(DirectX::XMFLOAT3(width / -2.0f, height + 0.15f, length)));

			std::vector<unsigned int> indices =
			{
				0,4, 0,1, 0,2, 0,3, // Back lines
				4,1, 1,2, 2,3, 3,4, // Front rectangle
				5,6, 6,7, 7,5 // Top triangle
			};

			SetVertexBuffer(Resource::VertexBuffer::Get(gfx, typeName, vertices));
			SetIndexBuffer(Resource::IndexBuffer::Get(gfx, typeName, indices));
		}
		else
		{
			Primitive::IndexedTriangleList list;
			SetVertexBuffer(Resource::VertexBuffer::Get(gfx, typeName, list.vertices));
			SetIndexBuffer(Resource::IndexBuffer::Get(gfx, typeName, list.indices));
		}
		SetTopology(gfx, D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

		std::vector<std::shared_ptr<Pipeline::Technique>> techniques;
		techniques.reserve(4);
		auto material = std::make_shared<Visual::Material>(gfx, color, typeName + name);
		auto dimmedMaterial = std::make_shared<Visual::Material>(gfx, color * 0.75f, typeName + name + "Dim");
		auto vertexLayout = material->GerVertexLayout();

		techniques.emplace_back(Pipeline::TechniqueFactory::MakeShadowMap(gfx, graph, vertexLayout));
		techniques.emplace_back(Pipeline::TechniqueFactory::MakeLambertian(graph, RenderChannel::Main, std::move(material)));
		techniques.emplace_back(Pipeline::TechniqueFactory::MakeWireframe(graph, RenderChannel::Main, std::move(dimmedMaterial)));
		techniques.emplace_back(Pipeline::TechniqueFactory::MakeOutlineBlur(gfx, graph, RenderChannel::Main, name, std::move(vertexLayout)));
		SetTechniques(gfx, std::move(techniques), *this);
	}

	void CameraIndicator::Accept(Graphics& gfx, Probe::BaseProbe& probe) noexcept
	{
		Object::Accept(gfx, probe);
		BaseShape::Accept(gfx, probe);
	}
}