#include "SolidCone.h"
#include "Primitives.h"
#include "TechniqueFactory.h"
#include "Math.h"

namespace GFX::Shape
{
	SolidCone::SolidCone(Graphics& gfx, Pipeline::RenderGraph& graph, const DirectX::XMFLOAT3& position,
		const std::string& name, Data::ColorFloat3 color, unsigned int density, float height, float angle)
		: IShape(gfx, position, name), height(height), angle(Math::Wrap(angle, 90.0f))
	{
		std::string typeName = Primitive::Cone::GetNameSolid(density);
		if (Resource::VertexBuffer::NotStored(typeName) && Resource::IndexBuffer::NotStored(typeName))
		{
			auto list = Primitive::Cone::MakeSolid(density);
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
		materialBuffer = &material->GetPixelBuffer();

		techniques.emplace_back(Pipeline::TechniqueFactory::MakeShadowMap(gfx, graph, material));
		techniques.emplace_back(Pipeline::TechniqueFactory::MakeLambertian(gfx, graph, std::move(material)));
		techniques.emplace_back(Pipeline::TechniqueFactory::MakeOutlineBlur(gfx, graph, name, std::move(vertexLayout)));
		SetTechniques(gfx, std::move(techniques), *this);

		UpdateTransformMatrix();
	}

	void SolidCone::UpdateTransformMatrix() noexcept
	{
		const float circleScale = height * tanf(static_cast<float>(2.0 * M_PI - FLT_EPSILON) * angle / 361.0f);
		DirectX::XMStoreFloat4x4(transform.get(),
			DirectX::XMMatrixScaling(circleScale, height, circleScale) *
			CreateTransformMatrix());
	}
}