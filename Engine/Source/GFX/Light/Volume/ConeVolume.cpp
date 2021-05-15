#include "GFX/Light/Volume/ConeVolume.h"
#include "GFX/Primitive/Cone.h"

namespace ZE::GFX::Light::Volume
{
	void ConeVolume::UpdateTransform(float volume, const Data::CBuffer::DynamicCBuffer& lightBuffer) noexcept
	{
		const float circleScale = volume * tanf(static_cast<float>(lightBuffer["outerAngle"]) + 0.22f);
		Float3 pos = lightBuffer["lightPos"];
		pos.y -= volume;

		Math::XMStoreFloat4x4(transform.get(), Math::XMMatrixScaling(circleScale, volume, circleScale) *
			Math::GetVectorRotation(Math::XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f),
				Math::XMLoadFloat3(&lightBuffer["direction"]), true, volume) *
			Math::XMMatrixTranslationFromVector(Math::XMLoadFloat3(&pos)));
	}

	ConeVolume::ConeVolume(Graphics& gfx, U32 density)
		: IVolume(gfx)
	{
		std::string typeName = Primitive::Cone::GetNameSolid(density);
		if (Resource::VertexBuffer::NotStored(typeName) && Resource::IndexBuffer::NotStored(typeName))
		{
			auto list = Primitive::Cone::MakeSolid(density);
			vertexBuffer = Resource::VertexBuffer::Get(gfx, typeName, std::move(list.Vertices));
			indexBuffer = Resource::IndexBuffer::Get(gfx, typeName, std::move(list.Indices));
		}
		else
		{
			vertexBuffer = Resource::VertexBuffer::Get(gfx, typeName, {});
			indexBuffer = Resource::IndexBuffer::Get(gfx, typeName, {});
		}
	}
}