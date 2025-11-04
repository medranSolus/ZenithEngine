#include "MathExt.h"

namespace ZE::Math
{
	Float3 GetEulerAngles(const Float4& rotor) noexcept
	{
		constexpr float SINGULARITY = 0.4999f;
		Float3 euler;
		euler.x = rotor.w * rotor.x + rotor.y * rotor.z;

		if (euler.x > SINGULARITY) // South pole
		{
			euler.x = M_PI_2;
			euler.y = 2.0f * std::atan2(rotor.y, rotor.w);
			euler.z = 0.0f;
		}
		else if (euler.x < -SINGULARITY) // North pole
		{
			euler.x = -M_PI_2;
			euler.y = -2.0f * std::atan2(rotor.y, rotor.w);
			euler.z = 0.0f;
		}
		else
		{
			euler.x = std::asin(2.0f * euler.x);

			const float qx2 = rotor.x * rotor.x;
			euler.y = std::atan2(2.0f * (rotor.w * rotor.y - rotor.x * rotor.z),
				1 - 2.0f * (qx2 + rotor.y * rotor.y));
			euler.z = std::atan2(2.0f * (rotor.w * rotor.z - rotor.x * rotor.y),
				1 - 2.0f * (qx2 + rotor.z * rotor.z));
		}
		return euler;
	}

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

	bool IsUnitQuaternion(const Vector& rotor) noexcept
	{
		constexpr Vector EPSILON = { 1.0e-4f, 1.0e-4f, 1.0e-4f, 1.0e-4f };

		Vector diff = XMVectorSubtract(XMVector4Length(rotor), XMVectorSplatOne());
		return XMVector4Less(XMVectorAbs(diff), EPSILON);
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
				rotation *= XMMatrixRotationX(PI);
				angle -= PI;
			}
			rotation *= XMMatrixRotationNormal(XMVector3Cross(baseDirection, newDirection), angle);

			if (NotEquals(geometryOffsetY, 0.0f))
				rotation *= XMMatrixTranslation(0.0f, geometryOffsetY, 0.0f);
		}
		return rotation;
	}

	Matrix GetTransform(const Float3& position, const Float4& rotor, const Float3& scale) noexcept
	{
		return XMMatrixScalingFromVector(XMLoadFloat3(&scale)) *
			XMMatrixRotationQuaternion(XMLoadFloat4(&rotor)) *
			XMMatrixTranslationFromVector(XMLoadFloat3(&position));
	}

	BoundingBox GetBoundingBox(const Vector& maxPositive, const Vector& maxNegative) noexcept
	{
		BoundingBox box;
		Math::XMStoreFloat3(&box.Extents,
			Math::XMVectorScale(Math::XMVectorSubtract(maxPositive, maxNegative), 0.5f));
		Math::XMStoreFloat3(&box.Center,
			Math::XMVectorScale(Math::XMVectorAdd(maxPositive, maxNegative), 0.5f));
		return box;
	}
}