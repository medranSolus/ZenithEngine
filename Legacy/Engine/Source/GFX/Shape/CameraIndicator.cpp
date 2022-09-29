#include "GFX/Shape/CameraIndicator.h"
#include "GFX/Primitive/CameraFrame.h"
#include "GFX/Pipeline/TechniqueFactory.h"

namespace ZE::GFX::Shape
{
	CameraIndicator::CameraIndicator(Graphics& gfx, Pipeline::RenderGraph& graph,
		const Float3& position, std::string&& name, const ColorF3& color)
		: IShape(gfx, position, std::forward<std::string>(name))
	{
		constexpr const char* TYPE_NAME = Primitive::CameraFrame::GetNameIndicator();
		if (Resource::VertexBuffer::NotStored(TYPE_NAME) && Resource::IndexBuffer::NotStored(TYPE_NAME))
		{
			SetVertexBuffer(Resource::VertexBuffer::Get(gfx, TYPE_NAME, Primitive::CameraFrame::MakeIndicatorVertex()));
			SetIndexBuffer(Resource::IndexBuffer::Get(gfx, TYPE_NAME, Primitive::CameraFrame::MakeIndicatorIndex()));
		}
		else
		{
			SetVertexBuffer(Resource::VertexBuffer::Get(gfx, TYPE_NAME, {}));
			SetIndexBuffer(Resource::IndexBuffer::Get(gfx, TYPE_NAME, {}));
		}
		SetTopology(gfx, D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

		std::vector<Pipeline::Technique> techniques;
		techniques.reserve(4);
		auto material = std::make_shared<Visual::Material>(gfx, color, TYPE_NAME + GetName());
		auto dimmedMaterial = std::make_shared<Visual::Material>(gfx, color * 0.75f, TYPE_NAME + GetName() + "_D");
		auto vertexLayout = material->GerVertexLayout();

		techniques.emplace_back(Pipeline::TechniqueFactory::MakeShadowMap(gfx, graph, material));
		techniques.emplace_back(Pipeline::TechniqueFactory::MakeLambertian(gfx, graph, std::move(material)));
		techniques.emplace_back(Pipeline::TechniqueFactory::MakeWireframe(graph, std::move(dimmedMaterial)));
		techniques.emplace_back(Pipeline::TechniqueFactory::MakeOutlineBlur(gfx, graph, GetName(), std::move(vertexLayout)));
		SetTechniques(gfx, std::move(techniques), *this);
	}
}