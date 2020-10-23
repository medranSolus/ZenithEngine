#include "GlobeVolume.h"
#include "Primitives.h"

namespace GFX::Light::Volume
{
	void GlobeVolume::UpdateTransform(float volume, const Data::CBuffer::DynamicCBuffer& lightBuffer) noexcept
	{
		DirectX::XMStoreFloat4x4(transform.get(),
			DirectX::XMMatrixScaling(volume, volume, volume) *
			DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&lightBuffer["lightPos"])));
	}

	GlobeVolume::GlobeVolume(Graphics& gfx, unsigned int density)
		: IVolume(gfx)
	{
		std::string typeName = Primitive::Sphere::GetNameIcoSolid(density);
		if (Resource::VertexBuffer::NotStored(typeName) && Resource::IndexBuffer::NotStored(typeName))
		{
			auto list = Primitive::Sphere::MakeIcoSolid(density);
			vertexBuffer = Resource::VertexBuffer::Get(gfx, typeName, list.vertices);
			indexBuffer = Resource::IndexBuffer::Get(gfx, typeName, list.indices);
		}
		else
		{
			Primitive::IndexedTriangleList list;
			vertexBuffer = Resource::VertexBuffer::Get(gfx, typeName, list.vertices);
			indexBuffer = Resource::IndexBuffer::Get(gfx, typeName, list.indices);
		}
	}
}