#include "MathExt.h"

namespace ZE::Math
{
	Float2 EncodeOctahedral(const Float3& nrm) noexcept
	{
		// Project onto octahedron (L1 norm)
		Vector n = XMLoadFloat3(&nrm);
		Vector absN = XMVectorAbs(n);

		Float3 l1 = {};
		XMStoreFloat3(&l1, XMVectorDivide(n, XMVectorAdd(XMVectorSplatX(absN), XMVectorAdd(XMVectorSplatY(absN), XMVectorSplatZ(absN)))));
		
		// If z is negative, fold the coordinates across the axes
		if (l1.z < 0.0f)
			return { (1.0f - std::abs(l1.y)) * SignNotZero(l1.x), (1.0f - std::abs(l1.x)) * SignNotZero(l1.y) };
		else
			return { l1.x, l1.y };
	}

	Float3 DecodeOctahedral(const Float3& oct) noexcept
	{
		Float3 v = { oct.x, oct.y, 1.0f - std::abs(oct.x) - std::abs(oct.y) };

		if (v.z < 0.0f)
		{
			v.x -= v.z * SignNotZero(v.x);
			v.y -= v.z * SignNotZero(v.y);
		}

		XMStoreFloat3(&v, XMVector3Normalize(XMLoadFloat3(&v)));
		return v;
	}

	float EncodeTangentAngle(const Float3& nrm, const Float3& tangent) noexcept
	{
		Vector n = XMLoadFloat3(&nrm);
		Vector t = XMLoadFloat3(&tangent);

		// Canonical basis direction as in DOOM Eternal SIGGRAPH 2020 https://advances.realtimerendering.com/s2020/RenderingDoomEternal.pdf
		Vector axis = XMVector3Normalize(std::abs(nrm.x) > std::abs(nrm.z) ?
			XMVectorSet(-nrm.y, nrm.x, 0.0f, 0.0f) : XMVectorSet(0.0f, -nrm.z, nrm.y, 0.0f));

		Vector t1 = XMVector3Normalize(XMVector3Cross(axis, n));
		Vector t2 = XMVector3Normalize(XMVector3Cross(n, t1));

		// Get sin-cos after projection
		float c = XMVectorGetX(XMVector3Dot(t, t1));
		float s = XMVectorGetX(XMVector3Dot(t, t2));

		return std::atan2(s, c);
	}

	Float3 DecodeTangentAngle(const Float3& nrm, float angle) noexcept
	{
		Vector axis = XMVector3Normalize(std::abs(nrm.x) > std::abs(nrm.z) ?
			XMVectorSet(-nrm.y, nrm.x, 0.0f, 0.0f) : XMVectorSet(0.0f, -nrm.z, nrm.y, 0.0f));

		Float3 tangent = {};
		XMStoreFloat3(&tangent, XMVectorMultiplyAdd(axis, XMVectorReplicate(std::cos(angle)),
			XMVectorScale(XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&nrm), axis)), std::sin(angle))));
		return tangent;
	}

	float EncodeTangentDiamond(const Float3& nrm, const Float3& tangent) noexcept
	{
		// https://www.jeremyong.com/graphics/2023/01/09/tangent-spaces-and-diamond-encoding/#:~:text=%E2%80%9CDiamond%20Encoding
		Vector t = XMLoadFloat3(&tangent);

		// First canonical direction
		Vector t1 = XMVector3Normalize(std::abs(nrm.y) > std::abs(nrm.z) ?
			XMVectorSet(nrm.y, -nrm.x, 0.0f, 0.0f) : XMVectorSet(nrm.z, 0.0f, -nrm.x, 0.0f));

		// Construct t2 such that t1 and t2 span the plane
		Vector t2 = XMVector3Cross(t1, XMLoadFloat3(&nrm));

		// Decompose the tangent into two coordinates in the canonical basis
		Float2 packedTangent = { XMVectorGetX(XMVector3Dot(t, t1)), XMVectorGetX(XMVector3Dot(t, t2)) };

		// Project to the unit diamond, then to the x-axis.
		float x = packedTangent.x / (std::abs(packedTangent.x) + abs(packedTangent.y));

		// Contract the x coordinate by a factor of 4 to represent all 4 quadrants in
		// the unit range and remap
		float sign = Sign(packedTangent.y);
		return -sign * 0.25f * x + 0.5f + sign * 0.25f;
	}

	Float3 DecodeTangentDiamond(const Float3& nrm, float diamond) noexcept
	{
		// As in the encode step, find our canonical tangent basis span(t1, t2)
		Vector t1 = XMVector3Normalize(std::abs(nrm.y) > std::abs(nrm.z) ?
			XMVectorSet(nrm.y, -nrm.x, 0.0f, 0.0f) : XMVectorSet(nrm.z, 0.0f, -nrm.x, 0.0f));

		Vector t2 = XMVector3Cross(t1, XMLoadFloat3(&nrm));

		// Remap value to the appropriate segment on the diamond
		float sign = Sign(diamond - 0.5f);
		float x = -sign * 4.0f * diamond + 1.0f + sign * 2.0f;
		Vector packedTangent = { x, sign * (1.0f - std::abs(x)), 0.0f, 0.0f };

		// Normalization extends the point on the diamond back to the unit circle
		packedTangent = XMVector2Normalize(packedTangent);

		Float3 tangent = {};
		XMStoreFloat3(&tangent, XMVectorMultiplyAdd(t1, XMVectorSplatX(packedTangent), XMVectorMultiply(t2, XMVectorSplatY(packedTangent))));
		return tangent;
	}

	Float3 GetEulerAngles(const Float4& rotor) noexcept
	{
		constexpr float SINGULARITY = 0.4999f;
		Float3 euler = {};
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
		Float3 out = {};
		XMStoreFloat3(&out, XMVectorAdd(XMLoadFloat3(&v1), XMLoadFloat3(&v2)));
		return out;
	}

	Float3 AddNormal(const Float3& v1, const Float3& v2) noexcept
	{
		Float3 out = {};
		XMStoreFloat3(&out, XMVector3Normalize(XMVectorAdd(XMLoadFloat3(&v1), XMLoadFloat3(&v2))));
		return out;
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
		BoundingBox box = {};
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

	std::vector<float> GetFilterCoefficients(FilterType filter, U32 windowSize, float coeffParam) noexcept
	{
		std::vector<float> coeffs;

		switch (filter)
		{
		default:
			break;
		case Math::FilterType::Kaiser:
		case Math::FilterType::Lanczos:
		case Math::FilterType::Gauss:
		{
			coeffs.resize((windowSize >> 1) + (windowSize & 1));
			const U32 coeffSize = Utils::SafeCast<U32>(coeffs.size());

			float coeffSum = 0.0f;
			switch (filter)
			{
			case Math::FilterType::Kaiser:
			{
				if (coeffParam == 0.0f)
					coeffParam = 7.64f;

				for (U32 i = 0; i < coeffSize; ++i)
				{
					// Regular Kaiser window reaches 1 at length / 2 so need to scale it correctly
					const float k = Math::Kaiser(static_cast<float>(i + coeffSize), coeffParam, static_cast<float>(coeffSize * 2));
					coeffs.at(i) = k;
					coeffSum += k;
				}
				break;
			}
			case Math::FilterType::Lanczos:
			{
				for (U32 i = 0; i < coeffSize; ++i)
				{
					const float l = Math::Lanczos(static_cast<float>(i), static_cast<float>(coeffSize));
					coeffs.at(i) = l;
					coeffSum += l;
				}
				break;
			}
			case Math::FilterType::Gauss:
			{
				if (coeffParam == 0.0f)
					coeffParam = 2.6f;

				for (U32 i = 0; i < coeffSize; ++i)
				{
					const float g = Math::Gauss(static_cast<float>(i), coeffParam);
					coeffs.at(i) = g;
					coeffSum += g;
				}
				break;
			}
			default:
				break;
			}

			// Normalize filter coefficients, symetric kernel requires doubling the sum
			// but just for even windows, odd windows have center coeff counted once
			coeffSum *= 2.0f;
			if (windowSize & 1)
				coeffSum -= coeffs.front();

			for (float& coeff : coeffs)
				coeff /= coeffSum;
			break;
		}
		}

		return coeffs;
	}

	Vector ApplyFilter(FilterType filter, std::vector<Float4>& samples, float factorX, float factorY, const std::vector<float>* filterCoeff) noexcept
	{
		// https://bgolus.medium.com/sharper-mipmapping-using-shader-based-supersampling-ed7aadb47bec
		Vector result = {};
		switch (filter)
		{
		default:
			ZE_ENUM_UNHANDLED();
		case FilterType::Box:
		{
			for (const auto& sample : samples)
				result = XMVectorAdd(result, XMLoadFloat4(&sample));

			if (samples.size())
				result = XMVectorScale(result, 1.0f / static_cast<float>(samples.size()));
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
				result = XMVectorScale(XMVectorSqrt(result), 1.0f / static_cast<float>(samples.size()));
			break;
		}
		case FilterType::Bilinear:
		{
			ZE_ASSERT(samples.size() == 4, "Bilinear filter requires exactly 4 samples!");
			// Bilinear interpolation weights
			const Vector w1 = XMVectorReplicate((1.0f - factorY) * (1.0f - factorX));
			const Vector w2 = XMVectorReplicate(factorY * (1.0f - factorX));
			const Vector w3 = XMVectorReplicate((1.0f - factorY) * factorX);
			const Vector w4 = XMVectorReplicate(factorY * factorX);

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
			U32 windowSize = Utils::SafeCast<U32>((filterCoeff->size() << 1) - static_cast<U32>(!evenWindow));
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
		case FilterType::BicubicSharp:
		case FilterType::BicubicSmooth:
		{
			ZE_ASSERT(samples.size() == 16, "Bicubic filter requires exactly 16 samples!");
			const bool sharp = filter == FilterType::BicubicSharp;

			float coeffSum = 0.0f;
			U8 i = 0;
			for (S32 dy = -1; dy <= 2; ++dy)
			{
				float y = factorY - static_cast<float>(dy);
				float weightY = sharp ? Math::CatmullRom(y) : Math::MitchellNetravali(y);

				for (S32 dx = -1; dx <= 2; ++dx)
				{
					float x = factorX - static_cast<float>(dx);
					float weight = (sharp ? Math::CatmullRom(x) : Math::MitchellNetravali(x)) * weightY;
					coeffSum += weight;

					result = Math::XMVectorMultiplyAdd(Math::XMLoadFloat4(&samples.at(i++)), Math::XMVectorReplicate(weight), result);
				}
			}

			result = XMVectorScale(result, 1.0f / coeffSum);
			break;
		}
		}
		return result;
	}
}