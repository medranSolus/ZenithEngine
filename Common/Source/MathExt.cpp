#include "MathExt.h"

namespace Math
{
	Float3 Add(const Float3& v1, const Float3& v2) noexcept
	{
		Float3 out;
		XMStoreFloat3(&out, XMVectorAdd(XMLoadFloat3(&v1), XMLoadFloat3(&v2)));
		return std::move(out);
	}

	Float3 AddNormal(const Float3& v1, const Float3& v2) noexcept
	{
		Float3 out;
		XMStoreFloat3(&out, XMVector3Normalize(XMVectorAdd(XMLoadFloat3(&v1), XMLoadFloat3(&v2))));
		return std::move(out);
	}

	bool IsNearEqual(const Vector& v1, const Vector& v2) noexcept
	{
		const Vector equality = XMVectorNearEqual(v1, v2, XMVectorSet(FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, 0.0f));
		return std::isnan(XMVectorGetX(equality)) && std::isnan(XMVectorGetY(equality)) && std::isnan(XMVectorGetZ(equality));
	}

	Matrix GetVectorRotation(const Vector& baseDirection, const Vector& newDirection,
		bool targetGeometry, float geometryOffsetY) noexcept
	{
		Matrix rotation = XMMatrixIdentity();

		// Check if direction vectors are not near equal (cross product restriction) to perform aligning to new vector
		if (!IsNearEqual(baseDirection, newDirection))
		{
			if (NotEquals(geometryOffsetY, 0.0f))
				rotation *= XMMatrixTranslation(0.0f, -geometryOffsetY, 0.0f);

			float angle = XMVectorGetX(XMVector3AngleBetweenNormals(baseDirection, newDirection));
			// When transforming over 90 degrees geometry gets flattened in X axis without this
			if (targetGeometry && angle > M_PI_2)
			{
				rotation *= XMMatrixRotationX(static_cast<float>(M_PI));
				angle -= static_cast<float>(M_PI);
			}
			rotation *= XMMatrixRotationNormal(XMVector3Cross(baseDirection, newDirection), angle);

			if (NotEquals(geometryOffsetY, 0.0f))
				rotation *= XMMatrixTranslation(0.0f, geometryOffsetY, 0.0f);
		}
		return rotation;
	}
}