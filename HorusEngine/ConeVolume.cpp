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

		const DirectX::XMVECTOR direction = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&lightBuffer["direction"]));
		const DirectX::XMVECTOR geometryOrienation = DirectX::XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f);
		DirectX::XMMATRIX volumeMatrix = DirectX::XMMatrixScaling(circleScale, volume, circleScale);

		const DirectX::XMVECTOR equality = DirectX::XMVectorNearEqual(geometryOrienation, direction,
			DirectX::XMVectorSet(FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, 0.0f));
		// Check if direction vectors are not near equal (cross product restriction) to perform aligning to new geometry
		if (!std::isnan(DirectX::XMVectorGetX(equality)) || !std::isnan(DirectX::XMVectorGetY(equality)) || !std::isnan(DirectX::XMVectorGetZ(equality)))
		{
			volumeMatrix *= DirectX::XMMatrixTranslation(0.0f, -volume, 0.0f);

			float angle = DirectX::XMVectorGetX(DirectX::XMVector3AngleBetweenNormals(geometryOrienation, direction));
			// When transforming over 90 degrees cone gets flattened in X axis without this
			if (angle > M_PI_2)
			{
				volumeMatrix *= DirectX::XMMatrixRotationX(M_PI);
				angle -= M_PI;
			}
			volumeMatrix *= DirectX::XMMatrixRotationNormal(DirectX::XMVector3Cross(geometryOrienation, direction), angle) *
				DirectX::XMMatrixTranslation(0.0f, volume, 0.0f);
		}
		DirectX::XMStoreFloat4x4(transform.get(), volumeMatrix *
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