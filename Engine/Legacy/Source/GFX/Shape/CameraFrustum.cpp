#include "GFX/Shape/CameraFrustum.h"
#include "GFX/Primitive/CameraFrame.h"
#include "GFX/Pipeline/TechniqueFactory.h"

namespace ZE::GFX::Shape
{
	CameraFrustum::CameraFrustum(Graphics& gfx, Pipeline::RenderGraph& graph, const Float3& position,
		std::string&& name, const ColorF3& color, const Camera::ProjectionData& data)
		: IShape(gfx, position, std::forward<std::string>(name))
	{
		constexpr const char* TYPE_NAME = Primitive::CameraFrame::GetNameFrustum();
		if (Resource::IndexBuffer::NotStored(TYPE_NAME))
		{
			SetIndexBuffer(Resource::IndexBuffer::Get(gfx,
				TYPE_NAME, Primitive::CameraFrame::MakeFrustumIndex()));
		}
		else
			SetIndexBuffer(Resource::IndexBuffer::Get(gfx, TYPE_NAME, {}));

		SetParams(gfx, data);
		SetTopology(gfx, D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

		std::vector<Pipeline::Technique> techniques;
		techniques.reserve(3);
		const std::string tag = TYPE_NAME + GetName();
		auto material = std::make_shared<Visual::Material>(gfx, color, tag);
		auto dimmedMaterial = std::make_shared<Visual::Material>(gfx, color * 0.75f, tag + "_D");

		techniques.emplace_back(Pipeline::TechniqueFactory::MakeShadowMap(gfx, graph, material));
		techniques.emplace_back(Pipeline::TechniqueFactory::MakeLambertian(gfx, graph, std::move(material)));
		techniques.emplace_back(Pipeline::TechniqueFactory::MakeWireframe(graph, std::move(dimmedMaterial)));
		SetTechniques(gfx, std::move(techniques), *this);
	}

	void CameraFrustum::SetParams(Graphics& gfx, const Camera::ProjectionData& data)
	{
		SetVertexBuffer(GfxResPtr<Resource::VertexBuffer>(gfx,
			Primitive::CameraFrame::GetNameFrustum() + GetName(), Primitive::CameraFrame::MakeFrustumVertex(data)));
	}
}