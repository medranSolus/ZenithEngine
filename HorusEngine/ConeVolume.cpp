#include "ConeVolume.h"
#include "Primitives.h"
#include "Math.h"

namespace GFX::Light::Volume
{
	void ConeVolume::UpdateTransform(float volume, const Data::CBuffer::DynamicCBuffer& lightBuffer) noexcept
	{
		const float circleScale = volume * tanf(static_cast<float>(lightBuffer["outerAngle"]) + 0.22f);
		DirectX::XMFLOAT3 pos = lightBuffer["lightPos"];
		pos.y -= volume;

		DirectX::XMStoreFloat4x4(transform.get(), DirectX::XMMatrixScaling(circleScale, volume, circleScale) *
			Math::GetVectorRotation(DirectX::XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f),
				DirectX::XMLoadFloat3(&lightBuffer["direction"]), true, volume) *
			DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&pos)));
	}

	ConeVolume::ConeVolume(Graphics& gfx, unsigned int density)
		: IVolume(gfx)
	{
		std::string typeName = Primitive::Cone::GetNameSolid(density);
		if (Resource::VertexBuffer::NotStored(typeName) && Resource::IndexBuffer::NotStored(typeName))
		{
			auto list = Primitive::Cone::MakeSolid(density);
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