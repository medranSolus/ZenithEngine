#include "Math.h"

namespace Math
{
	DirectX::XMFLOAT3 Add(const DirectX::XMFLOAT3& v1, const DirectX::XMFLOAT3& v2) noexcept
	{
		DirectX::XMFLOAT3 out;
		DirectX::XMStoreFloat3(&out, DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&v1), DirectX::XMLoadFloat3(&v2)));
		return std::move(out);
	}

	DirectX::XMFLOAT3 AddNormal(const DirectX::XMFLOAT3& v1, const DirectX::XMFLOAT3& v2) noexcept
	{
		DirectX::XMFLOAT3 out;
		DirectX::XMStoreFloat3(&out, DirectX::XMVector3Normalize(DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&v1), DirectX::XMLoadFloat3(&v2))));
		return std::move(out);
	}

	bool IsNearEqual(const DirectX::XMVECTOR& v1, const DirectX::XMVECTOR& v2) noexcept
	{
		const DirectX::XMVECTOR equality = DirectX::XMVectorNearEqual(v1, v2,
			DirectX::XMVectorSet(FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, 0.0f));

		return std::isnan(DirectX::XMVectorGetX(equality)) && std::isnan(DirectX::XMVectorGetY(equality)) && std::isnan(DirectX::XMVectorGetZ(equality));
	}

	DirectX::XMMATRIX GetVectorRotation(const DirectX::XMVECTOR& baseDirection, const DirectX::XMVECTOR& newDirection,
		bool targetGeometry, float geometryOffsetY) noexcept
	{
		DirectX::XMMATRIX rotation = DirectX::XMMatrixIdentity();

		// Check if direction vectors are not near equal (cross product restriction) to perform aligning to new vector
		if (!Math::IsNearEqual(baseDirection, newDirection))
		{
			if (NotEquals(geometryOffsetY, 0.0f))
				rotation *= DirectX::XMMatrixTranslation(0.0f, -geometryOffsetY, 0.0f);

			float angle = DirectX::XMVectorGetX(DirectX::XMVector3AngleBetweenNormals(baseDirection, newDirection));
			// When transforming over 90 degrees geometry gets flattened in X axis without this
			if (targetGeometry && angle > M_PI_2)
			{
				rotation *= DirectX::XMMatrixRotationX(M_PI);
				angle -= M_PI;
			}
			rotation *= DirectX::XMMatrixRotationNormal(DirectX::XMVector3Cross(baseDirection, newDirection), angle);

			if (NotEquals(geometryOffsetY, 0.0f))
				rotation *= DirectX::XMMatrixTranslation(0.0f, geometryOffsetY, 0.0f);
		}

		return rotation;
	}
}