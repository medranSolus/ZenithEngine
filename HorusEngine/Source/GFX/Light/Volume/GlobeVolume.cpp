#include "GFX/Light/Volume/GlobeVolume.h"
#include "GFX/Primitive/Sphere.h"

namespace GFX::Light::Volume
{
	void GlobeVolume::UpdateTransform(float volume, const Data::CBuffer::DynamicCBuffer& lightBuffer) noexcept
	{
		Math::XMStoreFloat4x4(transform.get(),
			Math::XMMatrixScaling(volume, volume, volume) *
			Math::XMMatrixTranslationFromVector(Math::XMLoadFloat3(&lightBuffer["lightPos"])));
	}

	GlobeVolume::GlobeVolume(Graphics& gfx, U32 density)
		: IVolume(gfx)
	{
		std::string typeName = Primitive::Sphere::GetNameIcoSolid(density);
		if (Resource::VertexBuffer::NotStored(typeName) && Resource::IndexBuffer::NotStored(typeName))
		{
			auto list = Primitive::Sphere::MakeIcoSolid(density);
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