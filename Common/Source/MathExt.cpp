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
		const Vector equality = XMVectorNearEqual(v1, v2, XMVectorReplicate(FLT_EPSILON));
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
		XMStoreFloat3(&box.Extents,
			XMVectorScale(XMVectorSubtract(maxPositive, maxNegative), 0.5f));
		XMStoreFloat3(&box.Center,
			XMVectorScale(XMVectorAdd(maxPositive, maxNegative), 0.5f));
		return box;
	}

	UInt3 SampleCubemap(const Vector& direction, U32 cubemapSize) noexcept
	{
		U32 sampleFace = UINT32_MAX;
		Float2 uv = SampleCubemapUV(direction, sampleFace);

		return { Utils::SafeCast<U32>(uv.x * static_cast<float>(cubemapSize - 1)), Utils::SafeCast<U32>(uv.y * static_cast<float>(cubemapSize - 1)), sampleFace };
	}

	Float2 SampleCubemapUV(const Vector& direction, U32& faceIndex) noexcept
	{
		const float sampleX = XMVectorGetX(direction);
		const float sampleY = XMVectorGetY(direction);
		const float sampleZ = XMVectorGetZ(direction);

		// Find which face to sample from
		Vector sampleAbs = XMVectorAbs(direction);

		Vector uvCoords = {};
		float uvFactor = 0.5f;
		if (XMComparisonAllTrue(XMVector3GreaterOrEqualR(XMVectorSplatY(sampleAbs), sampleAbs)))
		{
			uvFactor /= XMVectorGetY(sampleAbs);

			float coordY = 0.0f;
			if (sampleY >= 0.0f)
			{
				faceIndex = 2;
				coordY = sampleZ;
			}
			else
			{
				faceIndex = 3;
				coordY = -sampleZ;
			}
			uvCoords = XMVectorSet(sampleX, coordY, 0.0f, 0.0f);
		}
		else
		{
			float coordX = 0.0f;
			if (XMComparisonAllTrue(XMVector3GreaterOrEqualR(XMVectorSplatX(sampleAbs), sampleAbs)))
			{
				uvFactor /= XMVectorGetX(sampleAbs);

				if (sampleX >= 0.0f)
				{
					faceIndex = 0;
					coordX = -sampleZ;
				}
				else
				{
					faceIndex = 1;
					coordX = sampleZ;
				}
			}
			else
			{
				uvFactor /= XMVectorGetZ(sampleAbs);

				if (sampleZ >= 0.0f)
				{
					faceIndex = 4;
					coordX = sampleX;
				}
				else
				{
					faceIndex = 5;
					coordX = -sampleX;
				}
			}
			uvCoords = XMVectorSet(coordX, -sampleY, 0.0f, 0.0f);
		}
		uvCoords = XMVectorMultiplyAdd(uvCoords, XMVectorReplicate(uvFactor), XMVectorReplicate(0.5f));

		Float2 uv = {};
		XMStoreFloat2(&uv, uvCoords);
		return uv;
	}

	Vector ApplyFilter(FilterType filter, std::vector<Float4>& samples, float bilinearFactorX, float bilinearFactorY, const std::vector<float>* filterCoeff) noexcept
	{
		// https://bgolus.medium.com/sharper-mipmapping-using-shader-based-supersampling-ed7aadb47bec
		Vector result = {};
		switch (filter)
		{
		case FilterType::Box:
		{
			for (const auto& sample : samples)
				result = XMVectorAdd(result, XMLoadFloat4(&sample));

			if (samples.size())
				result = XMVectorMultiply(result, XMVectorReplicate(1.0f / static_cast<float>(samples.size())));
			break;
		}
		case FilterType::GammaAverage:
		{
			for (const auto& sample : samples)
			{
				Vector v = XMLoadFloat4(&sample);
				result = XMVectorMultiplyAdd(v, v, result);
			}
			if (samples.size())
				result = XMVectorMultiply(XMVectorSqrt(result), XMVectorReplicate(1.0f / static_cast<float>(samples.size())));
			break;
		}
		case FilterType::Bilinear:
		{
			ZE_ASSERT(samples.size() == 4, "Bilinear filter requires exactly 4 samples!");
			// Bilinear interpolation weights
			const Vector w1 = XMVectorReplicate((1.0f - bilinearFactorY) * (1.0f - bilinearFactorX));
			const Vector w2 = XMVectorReplicate(bilinearFactorY * (1.0f - bilinearFactorX));
			const Vector w3 = XMVectorReplicate((1.0f - bilinearFactorY) * bilinearFactorX);
			const Vector w4 = XMVectorReplicate(bilinearFactorY * bilinearFactorX);

			const Vector v1 = XMLoadFloat4(&samples.at(0));
			const Vector v2 = XMLoadFloat4(&samples.at(1));
			const Vector v3 = XMLoadFloat4(&samples.at(2));
			const Vector v4 = XMLoadFloat4(&samples.at(3));

			result = XMVectorAbs(XMVectorMultiplyAdd(v1, w1, XMVectorMultiplyAdd(v2, w2, XMVectorMultiplyAdd(v3, w3, XMVectorMultiply(v4, w4)))));
			break;
		}
		case FilterType::Kaiser:
		case FilterType::Lanczos:
		case FilterType::Gauss:
		{
			bool evenWindow = (samples.size() & 1) == 0;
			U32 windowSize = Utils::SafeCast<U32>((filterCoeff->size() << 1) - (filterCoeff->size() & 1) - static_cast<U32>(!evenWindow));
			ZE_ASSERT(filterCoeff != nullptr, "Filter coefficients must be provided for this filter type!");
			ZE_ASSERT(windowSize * windowSize == samples.size(), "Not enough filter coefficients provided!");

			std::vector<Float4> rowSums;
			rowSums.reserve(windowSize);
			bool flipped0 = false;
			for (S32 i = -Utils::SafeCast<S32>(filterCoeff->size() - 1); const auto& sample : samples)
			{
				result = XMVectorMultiplyAdd(XMLoadFloat4(&sample), XMVectorReplicate(filterCoeff->at(std::abs(i))), result);
				if (evenWindow)
				{
					if (i == 0 && !flipped0)
					{
						flipped0 = true;
						continue;
					}
				}
				if (static_cast<U64>(++i) == filterCoeff->size())
				{
					i = -Utils::SafeCast<S32>(filterCoeff->size() - 1);
					XMStoreFloat4(&rowSums.emplace_back(), result);
					result = XMVectorZero();
					flipped0 = false;
				}
			}
			result = XMVectorZero();
			for (S32 i = -Utils::SafeCast<S32>(filterCoeff->size() - 1); const auto& rowSum : rowSums)
			{
				result = XMVectorMultiplyAdd(XMLoadFloat4(&rowSum), XMVectorReplicate(filterCoeff->at(std::abs(i++))), result);
				if (evenWindow)
				{
					if (i == 1 && !flipped0)
					{
						flipped0 = true;
						--i;
					}
				}
			}
			break;
		}
		default:
		break;
		}
		return result;
	}
}