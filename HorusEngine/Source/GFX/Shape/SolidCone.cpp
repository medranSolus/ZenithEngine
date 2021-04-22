#include "GFX/Shape/SolidCone.h"
#include "GFX/Primitive/Cone.h"
#include "GFX/Pipeline/TechniqueFactory.h"

namespace GFX::Shape
{
	SolidCone::SolidCone(Graphics& gfx, Pipeline::RenderGraph& graph, const Float3& position,
		std::string&& name, const ColorF3& color, U32 density, float height, float angle)
		: IShape(gfx, position, std::forward<std::string>(name)),
		height(height), angle(Math::Wrap(angle, 90.0f))
	{
		std::string typeName = Primitive::Cone::GetNameSolid(density);
		if (Resource::VertexBuffer::NotStored(typeName) && Resource::IndexBuffer::NotStored(typeName))
		{
			auto list = Primitive::Cone::MakeSolid(density);
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
		materialBuffer = &material->GetPixelBuffer();

		techniques.emplace_back(Pipeline::TechniqueFactory::MakeShadowMap(gfx, graph, material));
		techniques.emplace_back(Pipeline::TechniqueFactory::MakeLambertian(gfx, graph, std::move(material)));
		techniques.emplace_back(Pipeline::TechniqueFactory::MakeOutlineBlur(gfx, graph, GetName(), std::move(vertexLayout)));
		SetTechniques(gfx, std::move(techniques), *this);

		UpdateTransformMatrix();
	}

	void SolidCone::UpdateTransformMatrix() noexcept
	{
		const float circleScale = height * tanf(static_cast<float>(2.0 * M_PI - FLT_EPSILON) * angle / 361.0f);
		Math::XMStoreFloat4x4(transform.get(),
			Math::XMMatrixScaling(circleScale, height, circleScale) *
			CreateTransformMatrix());
	}
}