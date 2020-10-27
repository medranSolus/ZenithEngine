#include "CameraFrustum.h"
#include "Primitives.h"
#include "TechniqueFactory.h"

namespace GFX::Shape
{
	CameraFrustum::CameraFrustum(Graphics& gfx, Pipeline::RenderGraph& graph, const DirectX::XMFLOAT3& position,
		const std::string& name, Data::ColorFloat3 color, const Camera::ProjectionData& data)
		: IShape(gfx, position, name)
	{
		const std::string typeName = typeid(CameraFrustum).name();
		if (Resource::IndexBuffer::NotStored(typeName))
		{
			std::vector<unsigned int> indices =
			{
				0,1, 1,2, 2,3, 3,0, // Back rectangle
				4,5, 5,6, 6,7, 7,4, // Front rectangle
				0,4, 1,5, 2,6, 3,7  // Rear lines
			};
			SetIndexBuffer(Resource::IndexBuffer::Get(gfx, typeName, indices));
		}
		else
			SetIndexBuffer(Resource::IndexBuffer::Get(gfx, typeName, {}));
		SetParams(gfx, data);
		SetTopology(gfx, D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

		std::vector<std::shared_ptr<Pipeline::Technique>> techniques;
		techniques.reserve(3);
		auto material = std::make_shared<Visual::Material>(gfx, color, typeName + name);
		auto dimmedMaterial = std::make_shared<Visual::Material>(gfx, color * 0.75f, typeName + name + "Dim");

		techniques.emplace_back(Pipeline::TechniqueFactory::MakeShadowMap(gfx, graph, material));
		techniques.emplace_back(Pipeline::TechniqueFactory::MakeLambertian(gfx, graph, std::move(material)));
		techniques.emplace_back(Pipeline::TechniqueFactory::MakeWireframe(graph, std::move(dimmedMaterial)));
		SetTechniques(gfx, std::move(techniques), *this);
	}

	void CameraFrustum::SetParams(Graphics& gfx, const Camera::ProjectionData& data)
	{
		Data::VertexBufferData vertices(std::make_shared<Data::VertexLayout>(), 8U);
		const float zRatio = data.farClip / data.nearClip;
		const float nearY = data.nearClip * tanf(data.fov / 2.0f);
		const float nearX = nearY * data.screenRatio;
		const float farY = nearY * zRatio, farX = nearX * zRatio;
		vertices.SetBox({ farY, -farY, -farX, farX, data.nearClip, data.farClip });

		vertices[0].SetByIndex(0, std::move(DirectX::XMFLOAT3(-nearX, nearY, data.nearClip)));
		vertices[1].SetByIndex(0, std::move(DirectX::XMFLOAT3(nearX, nearY, data.nearClip)));
		vertices[2].SetByIndex(0, std::move(DirectX::XMFLOAT3(nearX, -nearY, data.nearClip)));
		vertices[3].SetByIndex(0, std::move(DirectX::XMFLOAT3(-nearX, -nearY, data.nearClip)));
		vertices[4].SetByIndex(0, std::move(DirectX::XMFLOAT3(-farX, farY, data.farClip)));
		vertices[5].SetByIndex(0, std::move(DirectX::XMFLOAT3(farX, farY, data.farClip)));
		vertices[6].SetByIndex(0, std::move(DirectX::XMFLOAT3(farX, -farY, data.farClip)));
		vertices[7].SetByIndex(0, std::move(DirectX::XMFLOAT3(-farX, -farY, data.farClip)));

		SetVertexBuffer(std::make_shared<Resource::VertexBuffer>(gfx, typeid(CameraFrustum).name() + name, vertices));
	}
}