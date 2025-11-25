#include "GFX/Primitive.h"

typedef std::pair<U32, U32> Key;
namespace std
{
	// Hasher for Ico Sphere Generation
	template<> struct hash<Key>
	{
		static constexpr U32 Rotate(U32 x, U32 r) noexcept { return (x << r) | (x >> (32 - r)); }

		size_t operator()(Key const& k) const noexcept
		{
			U32 h1 = static_cast<U32>(std::hash<U32>{}(k.first));
			U32 h2 = static_cast<U32>(std::hash<U32>{}(k.second));
			// Inspired by https://www.boost.org/doc/libs/1_75_0/boost/container_hash/hash.hpp

			h2 *= 0xCC9E2D51;
			h2 = Rotate(h2, 15);
			h2 *= 0x1B873593;

			h1 ^= h2;
			h1 = Rotate(h1, 13);
			h1 = h1 * 5 + 0xE6546B64;
			return h1 ^ (ZE::Utils::SafeCast<size_t>(h2) << 1);
		}
	};
}

namespace ZE::GFX::Primitive
{
	template<bool GetSurfaceNormal, bool GetTangent, typename I>
	constexpr void ComputeGeometryData(std::vector<Vertex>& vertices, const std::vector<I>& indices) noexcept
	{
		static_assert(GetSurfaceNormal || GetTangent, "At least one of compute options should be specified!");
		ZE_ASSERT(indices.size() % 3 == 0 && indices.size() > 2, "Incorrect index data!");

		// https://gamedev.stackexchange.com/questions/68612/how-to-compute-tangent-and-bitangent-vectors
		for (U64 i = 0; i < indices.size(); i += 3)
		{
			Vertex& v0 = vertices.at(indices.at(i));
			Vertex& v1 = vertices.at(indices.at(i + 1));
			Vertex& v2 = vertices.at(indices.at(i + 2));

			const Vector p0 = Math::XMLoadFloat3(&v0.Position);
			const Vector edge1 = Math::XMVectorSubtract(Math::XMLoadFloat3(&v1.Position), p0);
			const Vector edge2 = Math::XMVectorSubtract(Math::XMLoadFloat3(&v2.Position), p0);

			// Compute surface normal
			Vector normal;
			if constexpr (GetSurfaceNormal)
			{
				normal = Math::XMVector3Normalize(Math::XMVector3Cross(edge1, edge2));

				Math::XMStoreFloat3(&v0.Normal, normal);
				Math::XMStoreFloat3(&v1.Normal, normal);
				Math::XMStoreFloat3(&v2.Normal, normal);
			}
			else
			{
				ZE_ASSERT(Math::Equals(Math::XMVectorGetX(Math::XMVector3Length(Math::XMLoadFloat3(&v0.Normal))), 1.0f) &&
					Math::Equals(Math::XMVectorGetX(Math::XMVector3Length(Math::XMLoadFloat3(&v1.Normal))), 1.0f) &&
					Math::Equals(Math::XMVectorGetX(Math::XMVector3Length(Math::XMLoadFloat3(&v2.Normal))), 1.0f),
					"Mesh don't have proper normals!");
			}

			if constexpr (GetTangent)
			{
				Float2 deltaUV1;
				Float2 deltaUV2;
				const Vector uv0 = Math::XMLoadFloat2(&v0.UV);
				Math::XMStoreFloat2(&deltaUV1, Math::XMVectorSubtract(Math::XMLoadFloat2(&v1.UV), uv0));
				Math::XMStoreFloat2(&deltaUV2, Math::XMVectorSubtract(Math::XMLoadFloat2(&v2.UV), uv0));

				const float direction = Math::Sign(deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
				ZE_ASSERT(direction != 0.0f, "Incorrect UV coordinates!");

				Vector tangent = Math::XMVectorScale(Math::XMVectorSubtract(
					Math::XMVectorScale(edge1, deltaUV2.y),
					Math::XMVectorScale(edge2, deltaUV1.y)), direction);

				// In case bitangent computation would be needed too
				//Vector bitangent = Math::XMVectorScale(Math::XMVectorSubtract(
				//	Math::XMVectorScale(edge2, deltaUV1.x),
				//	Math::XMVectorScale(edge1, deltaUV2.x)), direction);
				//
				//Vector bitan = Math::XMVectorSubtract(
				//	Math::XMVectorSubtract(bitangent, Math::XMVectorMultiply(normal, Math::XMVectorMultiply(bitangent, normal))),
				//	Math::XMVectorMultiply(localTan, Math::XMVectorMultiply(bitangent, localTan)));

				if constexpr (GetSurfaceNormal)
				{
					Math::XMStoreFloat3(reinterpret_cast<Float3*>(&v0.Tangent), Math::XMVector3Normalize(tangent));
					Math::XMStoreFloat3(reinterpret_cast<Float3*>(&v1.Tangent), Math::XMVector3Normalize(tangent));
					Math::XMStoreFloat3(reinterpret_cast<Float3*>(&v2.Tangent), Math::XMVector3Normalize(tangent));
				}
				else
				{
					normal = Math::XMLoadFloat3(&v0.Normal);
					Math::XMStoreFloat3(reinterpret_cast<Float3*>(&v0.Tangent),
						Math::XMVector3Normalize(Math::XMVectorSubtract(tangent,
							Math::XMVectorMultiply(normal, Math::XMVector3Dot(tangent, normal)))));

					normal = Math::XMLoadFloat3(&v1.Normal);
					Math::XMStoreFloat3(reinterpret_cast<Float3*>(&v1.Tangent),
						Math::XMVector3Normalize(Math::XMVectorSubtract(tangent,
							Math::XMVectorMultiply(normal, Math::XMVector3Dot(tangent, normal)))));

					normal = Math::XMLoadFloat3(&v2.Normal);
					Math::XMStoreFloat3(reinterpret_cast<Float3*>(&v2.Tangent),
						Math::XMVector3Normalize(Math::XMVectorSubtract(tangent,
							Math::XMVectorMultiply(normal, Math::XMVector3Dot(tangent, normal)))));
				}

				// Store handness
				v0.Tangent.w = direction;
				v1.Tangent.w = direction;
				v2.Tangent.w = direction;
			}
		}
	}

	void ComputeSurfaceNormalsTangents(std::vector<Vertex>& vertices, const std::vector<U32>& indices) noexcept
	{
		ComputeGeometryData<true, true>(vertices, indices);
	}

	void ComputeSurfaceNormalsTangents(std::vector<Vertex>& vertices, const std::vector<U16>& indices) noexcept
	{
		ComputeGeometryData<true, true>(vertices, indices);
	}

	void ComputeTangents(std::vector<Vertex>& vertices, const std::vector<U32>& indices) noexcept
	{
		ComputeGeometryData<false, true>(vertices, indices);
	}

	void ComputeTangents(std::vector<Vertex>& vertices, const std::vector<U16>& indices) noexcept
	{
		ComputeGeometryData<false, true>(vertices, indices);
	}

	namespace Square
	{
		std::vector<Float3> MakeSolidVertex() noexcept
		{
			constexpr float POINT = 0.5f;
			return
			{
				{ -POINT, -POINT, 0.0f },
				{ -POINT, POINT, 0.0f },
				{ POINT, POINT, 0.0f },
				{ POINT, -POINT, 0.0f }
			};
		}

		std::vector<U16> MakeSolidIndex() noexcept
		{
			return
			{
				0,1,2,
				0,2,3
			};
		}
	}
	namespace Cube
	{
		std::vector<Float3> MakeSolidVertex() noexcept
		{
			constexpr float POINT = 0.5f;
			return
			{
				{ -POINT, -POINT, -POINT },
				{ -POINT, POINT, -POINT },
				{ POINT, POINT, -POINT },
				{ POINT, -POINT, -POINT },
				{ -POINT, -POINT, POINT },
				{ -POINT, POINT, POINT },
				{ POINT, POINT, POINT },
				{ POINT, -POINT, POINT }
			};
		}

		std::vector<U16> MakeSolidIndex() noexcept
		{
			return
			{
				0,1,2, 0,2,3, // Front
				4,5,1, 4,1,0, // Left
				7,6,5, 7,5,4, // Back
				3,2,6, 3,6,7, // Right
				1,5,6, 1,6,2, // Top
				4,0,3, 4,3,7  // Down
			};
		}

		std::vector<U16> MakeSolidIndexInverted() noexcept
		{
			return
			{
				2,1,0, 3,2,0, // Front
				1,5,4, 0,1,4, // Left
				5,6,7, 4,5,7, // Back
				6,2,3, 7,6,3, // Right
				6,5,1, 2,6,1, // Top
				3,0,4, 7,3,4  // Down
			};
		}

		std::vector<Vertex> MakeVertex(const std::vector<U16>& indices) noexcept
		{
			ZE_ASSERT(indices.size() % 3 == 0 && indices.size() > 2, "Incorrect index data!");
			constexpr float POINT = 0.5f;
			constexpr float UV_SHORT_1 = 1.0f / 3.0f;
			constexpr float UV_SHORT_2 = 2.0f / 3.0f;

			/*
			* UV mapping according to:
			* 0,0       ________                1,0
			*    ______|  Top   |_______________
			*   | Left |  Back  | Right | Front |
			*    ------| Bottom |---------------
			* 0,1       --------                1,1
			*/
			std::vector<Vertex> vertices
			{
				// Front
				{ { -POINT, -POINT, -POINT }, {}, { 1.0f, UV_SHORT_2 } },
				{ { -POINT, POINT, -POINT }, {}, { 1.0f, UV_SHORT_1 } },
				{ { POINT, POINT, -POINT }, {}, { 0.75f, UV_SHORT_1 } },
				{ { POINT, -POINT, -POINT }, {}, { 0.75f, UV_SHORT_2 } },
				// Left
				{ { -POINT, -POINT, POINT }, {}, { 0.25f, UV_SHORT_2 } },
				{ { -POINT, POINT, POINT }, {}, { 0.25f, UV_SHORT_1 } },
				{ { -POINT, POINT, -POINT }, {}, { 0.0f, UV_SHORT_1 } },
				{ { -POINT, -POINT, -POINT }, {}, { 0.0f, UV_SHORT_2 } },
				// Back
				{ { POINT, -POINT, POINT }, {}, { 0.5f, UV_SHORT_2 } },
				{ { POINT, POINT, POINT }, {}, { 0.5f, UV_SHORT_1 } },
				{ { -POINT, POINT, POINT }, {}, { 0.25f, UV_SHORT_1 } },
				{ { -POINT, -POINT, POINT }, {}, { 0.25f, UV_SHORT_2 } },
				// Right
				{ { POINT, -POINT, -POINT }, {}, { 0.75f, UV_SHORT_2 } },
				{ { POINT, POINT, -POINT }, {}, { 0.75f, UV_SHORT_1 } },
				{ { POINT, POINT, POINT }, {}, { 0.5f, UV_SHORT_1 } },
				{ { POINT, -POINT, POINT }, {}, { 0.5f, UV_SHORT_2 } },
				// Top
				{ { -POINT, POINT, -POINT }, {}, { 0.25f, 0.0f } },
				{ { -POINT, POINT, POINT }, {}, { 0.25f, UV_SHORT_1 } },
				{ { POINT, POINT, POINT }, {}, { 0.5f, UV_SHORT_1 } },
				{ { POINT, POINT, -POINT }, {}, { 0.5f, 0.0f } },
				// Down
				{ { -POINT, -POINT, POINT }, {}, { 0.25f, UV_SHORT_2 } },
				{ { -POINT, -POINT, -POINT }, {}, { 0.25f, 1.0f } },
				{ { POINT, -POINT, -POINT }, {}, { 0.5f, 1.0f } },
				{ { POINT, -POINT, POINT }, {}, { 0.5f, UV_SHORT_2 } },
			};
			ComputeSurfaceNormalsTangents(vertices, indices);
			return vertices;
		}

		std::vector<U16> MakeIndex() noexcept
		{
			return
			{
				0,1,2,    0,2,3,    // Front
				4,5,6,    4,6,7,    // Left
				8,9,10,   8,10,11,  // Back
				12,13,14, 12,14,15, // Right
				16,17,18, 16,18,19, // Top
				20,21,22, 20,22,23  // Down
			};
		}
	}
	namespace Cone
	{
		Data<Float3, U16> MakeSolid(U16 density) noexcept
		{
			if (!density)
				density = 1;
			density *= 3;

			Data<Float3, U16> data;
			data.Vertices.resize(Utils::SafeCast<U64>(density) + 1);

			data.Vertices.at(0) = { 0.0f, 1.0f, 0.0f };
			data.Vertices.at(1) = { 0.0f, 0.0f, 0.5f }; // Base of circle

			const float angle = Math::PI2 / Utils::SafeCast<float>(density);
			const Vector base = Math::XMVectorSet(0.0f, 0.0f, 0.5f, 0.0f);

			for (U16 i = 1; i < density; ++i)
			{
				Math::XMStoreFloat3(&data.Vertices.at(i + 1),
					Math::XMVector3Transform(base, Math::XMMatrixRotationY(angle * Utils::SafeCast<float>(i))));
			}

			data.Indices.reserve(Utils::SafeCast<U64>(density - 1) * 6);
			for (U16 i = 2; i < density; ++i)
			{
				// Cone wall
				data.Indices.emplace_back(static_cast<U16>(0));
				data.Indices.emplace_back(i);
				data.Indices.emplace_back(static_cast<U16>(i + 1));
				// Cone base
				data.Indices.emplace_back(static_cast<U16>(1));
				data.Indices.emplace_back(static_cast<U16>(i + 1));
				data.Indices.emplace_back(i);
			}
			// First wall
			data.Indices.emplace_back(static_cast<U16>(0));
			data.Indices.emplace_back(static_cast<U16>(1));
			data.Indices.emplace_back(static_cast<U16>(2));
			// Last wall
			data.Indices.emplace_back(static_cast<U16>(0));
			data.Indices.emplace_back(density);
			data.Indices.emplace_back(static_cast<U16>(1));

			return data;
		}
	}
	namespace CameraFrame
	{
		std::vector<Float3> MakeIndicatorVertex() noexcept
		{
			constexpr float LENGTH = 1.0f, WIDTH = 1.0f, HEIGHT = 0.7f;
			return
			{
				{ 0.0f, 0.0f, 0.0f },
				{ -WIDTH, HEIGHT, LENGTH },
				{ WIDTH, HEIGHT, LENGTH },
				{ WIDTH, -HEIGHT, LENGTH },
				{ -WIDTH, -HEIGHT, LENGTH },
				{ WIDTH * 0.5f, HEIGHT + 0.15f, LENGTH },
				{ 0.0f, HEIGHT * 2.0f, LENGTH },
				{ WIDTH * -0.5f, HEIGHT + 0.15f, LENGTH }
			};
		}

		std::vector<U16> MakeIndicatorIndex() noexcept
		{
			return
			{
				0,4, 0,1, 0,2, 0,3, // Back lines
				4,1, 1,2, 2,3, 3,4, // Front rectangle
				5,6, 6,7, 7,5 // Top triangle
			};
		}

		Math::BoundingBox MakeFrustumBoundingBox(const ZE::Data::Projection& projection, float farClip) noexcept
		{
			const float zRatio = farClip / projection.NearClip;
			const float nearY = projection.NearClip * std::tanf(projection.FOV * 0.5f);
			const float nearX = nearY * projection.ViewRatio;
			const float farY = nearY * zRatio;
			const float farX = nearX * zRatio;

			if (farClip == FLT_MAX)
				farClip = projection.NearClip < 1.0f ? 1.0f / projection.NearClip : projection.NearClip * 1000.0f;

			const float xExtent = (farX - nearX) * 0.5f;
			const float yExtent = (farY - nearY) * 0.5f;
			const float zExtent = (farClip - projection.NearClip) * 0.5f;

			return { { farX - xExtent, farY - yExtent, farClip - zExtent }, { xExtent, yExtent, zExtent } };
		}

		std::vector<Float3> MakeFrustumVertex(const ZE::Data::Projection& projection, float farClip) noexcept
		{
			const float zRatio = farClip / projection.NearClip;
			const float nearY = projection.NearClip * std::tanf(projection.FOV * 0.5f);
			const float nearX = nearY * projection.ViewRatio;
			const float farY = nearY * zRatio;
			const float farX = nearX * zRatio;

			if (farClip == FLT_MAX)
				farClip = projection.NearClip < 1.0f ? 1.0f / projection.NearClip : projection.NearClip * 1000.0f;
			return
			{
				{ -nearX, nearY, projection.NearClip },
				{ nearX, nearY, projection.NearClip },
				{ nearX, -nearY, projection.NearClip },
				{ -nearX, -nearY, projection.NearClip },
				{ -farX, farY, farClip },
				{ farX, farY, farClip },
				{ farX, -farY, farClip },
				{ -farX, -farY, farClip }
			};
		}

		std::vector<U16> MakeFrustumIndex() noexcept
		{
			return
			{
				0,1, 1,2, 2,3, 3,0, // Back rectangle
				4,5, 5,6, 6,7, 7,4, // Front rectangle
				0,4, 1,5, 2,6, 3,7  // Rear lines
			};
		}
	}
	namespace Sphere
	{
		constexpr float RADIUS = 0.5f;

		constexpr float ICO_BIG_ANGLE = Math::PI * 0.1f;
		constexpr float ICO_SMALL_ANGLE = Math::PI * 0.3f;
		constexpr float ICO_BASE_ANGLE = 0.4636476090008061f; // std::atanf(0.5f);

		constexpr float ICO_CENTER_Z = RADIUS * 0.8944271910002763f; // std::cosf(ICO_BASE_ANGLE);
		constexpr float ICO_BIG_X = ICO_CENTER_Z * 0.9510565162951535f; // std::cosf(ICO_BIG_ANGLE);
		constexpr float ICO_BIG_Z = ICO_CENTER_Z * 0.3090169943749474f; // std::sinf(ICO_BIG_ANGLE);
		constexpr float ICO_SMALL_X = ICO_CENTER_Z * 0.5877852522924731f; // std::cosf(ICO_SMALL_ANGLE);
		constexpr float ICO_SMALL_Z = ICO_CENTER_Z * 0.8090169943749475f; // std::sinf(ICO_SMALL_ANGLE);
		constexpr float ICO_LEVEL = RADIUS * 0.44721359549923695f; // std::sinf(ICO_BASE_ANGLE);

		Data<Float3> MakeUVSolid(U32 latitudeDensity, U32 longitudeDensity) noexcept
		{
			if (!latitudeDensity)
				latitudeDensity = 1;
			if (!longitudeDensity)
				longitudeDensity = 1;
			latitudeDensity *= 3;
			longitudeDensity *= 3;

			const float latitudeAngle = Math::PI / Utils::SafeCast<float>(latitudeDensity);
			const float longitudeAngle = Math::PI2 / Utils::SafeCast<float>(longitudeDensity);
			const Vector base = Math::XMVectorSet(0.0f, RADIUS, 0.0f, 0.0f);

			Data<Float3> data;
			data.Vertices.reserve(Utils::SafeCast<U64>(latitudeDensity - 1) * longitudeDensity + 2);

			// Sphere vertices without poles
			for (U32 lat = 1, i = 0; lat < latitudeDensity; ++lat)
			{
				const Vector latBase = Math::XMVector3Transform(base, Math::XMMatrixRotationX(latitudeAngle * static_cast<float>(lat)));
				for (U32 lon = 0; lon < longitudeDensity; ++lon, ++i)
				{
					Math::XMStoreFloat3(&data.Vertices.at(i),
						Math::XMVector3Transform(latBase, Math::XMMatrixRotationY(longitudeAngle * static_cast<float>(lon))));
				}
			}
			const auto getIndex = [&longitudeDensity](U32 lat, U32 lon) constexpr -> U32
				{
					return lat * longitudeDensity + lon;
				};

			for (U32 lat = 0; lat < latitudeDensity - 2; ++lat)
			{
				for (U32 lon = 0; lon < longitudeDensity - 1; ++lon)
				{
					// Ring of rectangles around without last one
					U32 baseIndex = getIndex(lat, lon);
					data.Indices.emplace_back(baseIndex);
					data.Indices.emplace_back(baseIndex + longitudeDensity);
					data.Indices.emplace_back(baseIndex + longitudeDensity + 1);
					data.Indices.emplace_back(baseIndex);
					data.Indices.emplace_back(baseIndex + longitudeDensity + 1);
					data.Indices.emplace_back(baseIndex + 1);
				}

				// Last rectangle to connect ring
				U32 baseIndex = getIndex(lat, 0);
				data.Indices.emplace_back(baseIndex);
				data.Indices.emplace_back(baseIndex + longitudeDensity - 1);
				data.Indices.emplace_back(baseIndex + 2 * longitudeDensity - 1);
				data.Indices.emplace_back(baseIndex);
				data.Indices.emplace_back(baseIndex + 2 * longitudeDensity - 1);
				data.Indices.emplace_back(baseIndex + longitudeDensity);
			}

			// Poles vertices
			const U32 pole = Utils::SafeCast<U32>(data.Vertices.size() - 2);
			// South
			Math::XMStoreFloat3(&data.Vertices.at(pole), Math::XMVectorNegate(base));
			// North
			Math::XMStoreFloat3(&data.Vertices.at(pole + 1), base);

			// Triangle ring on each pole
			for (U32 lon = 0; lon < longitudeDensity - 1; ++lon)
			{
				// North
				U32 baseIndex = getIndex(0, lon);
				data.Indices.emplace_back(pole + 1);
				data.Indices.emplace_back(baseIndex);
				data.Indices.emplace_back(baseIndex + 1);
				// South
				baseIndex = getIndex(latitudeDensity - 2, lon);
				data.Indices.emplace_back(baseIndex + 1);
				data.Indices.emplace_back(baseIndex);
				data.Indices.emplace_back(pole);
			}

			// Last triangle to connect ring
			// North
			data.Indices.emplace_back(pole + 1);
			data.Indices.emplace_back(longitudeDensity - 1);
			data.Indices.emplace_back(0);
			// South
			U32 baseIndex = getIndex(latitudeDensity - 2, 0);
			data.Indices.emplace_back(baseIndex);
			data.Indices.emplace_back(baseIndex + longitudeDensity - 1);
			data.Indices.emplace_back(pole);

			return data;
		}

		Data<Float3> MakeIcoSolid(U32 density) noexcept
		{
			Data<Float3> data
			{
				{
					{ 0.0f, RADIUS, 0.0f },
					{ 0.0f, -RADIUS, 0.0f },
					{ 0.0f, ICO_LEVEL, -ICO_CENTER_Z },
					{ -ICO_BIG_X, ICO_LEVEL, -ICO_BIG_Z },
					{ -ICO_SMALL_X, ICO_LEVEL, ICO_SMALL_Z },
					{ ICO_SMALL_X, ICO_LEVEL, ICO_SMALL_Z },
					{ ICO_BIG_X, ICO_LEVEL, -ICO_BIG_Z },
					{ 0.0f, -ICO_LEVEL, ICO_CENTER_Z },
					{ ICO_BIG_X, -ICO_LEVEL, ICO_BIG_Z },
					{ ICO_SMALL_X, -ICO_LEVEL, -ICO_SMALL_Z },
					{ -ICO_SMALL_X, -ICO_LEVEL, -ICO_SMALL_Z },
					{ -ICO_BIG_X, -ICO_LEVEL, ICO_BIG_Z }
				},
				{
					// Triangle ring
					10,2,9,
					3,2,10,
					11,3,10,
					4,3,11,
					7,4,11,
					5,4,7,
					8,5,7,
					6,5,8,
					9,6,8,
					2,6,9,
					// North
					3,0,2,
					4,0,3,
					5,0,4,
					6,0,5,
					2,0,6,
					// South
					10,9,1,
					11,10,1,
					7,11,1,
					8,7,1,
					9,8,1
				}
			};

			auto comparator = [](const Key& x, const Key& y) constexpr -> bool
				{
					return (x.first == y.first && x.second == y.second) || (x.first == y.second && x.second == y.first);
				};
			for (U32 i = 0; i < density; ++i)
			{
				std::unordered_map<Key, U32, std::hash<Key>, decltype(comparator)> lookup(0, std::hash<Key>{}, comparator);
				std::vector<U32> tmpIndices;
				for (U64 j = 0; j < data.Indices.size(); j += 3)
				{
					auto i1 = lookup.find({ data.Indices.at(j), data.Indices.at(j + 1) });
					if (i1 == lookup.end())
					{
						i1 = lookup.emplace(std::make_pair(data.Indices.at(j), data.Indices.at(j + 1)), Utils::SafeCast<U32>(data.Vertices.size())).first;
						data.Vertices.emplace_back(Math::AddNormal(data.Vertices[data.Indices.at(j)], data.Vertices[data.Indices.at(j + 1)]));
					}
					auto i2 = lookup.find({ data.Indices.at(j + 1), data.Indices.at(j + 2) });
					if (i2 == lookup.end())
					{
						i2 = lookup.emplace(std::make_pair(data.Indices.at(j + 1), data.Indices.at(j + 2)), Utils::SafeCast<U32>(data.Vertices.size())).first;
						data.Vertices.emplace_back(Math::AddNormal(data.Vertices[data.Indices.at(j + 1)], data.Vertices[data.Indices.at(j + 2)]));
					}
					auto i3 = lookup.find({ data.Indices.at(j + 2), data.Indices.at(j) });
					if (i3 == lookup.end())
					{
						i3 = lookup.emplace(std::make_pair(data.Indices.at(j + 2), data.Indices.at(j)), Utils::SafeCast<U32>(data.Vertices.size())).first;
						data.Vertices.emplace_back(Math::AddNormal(data.Vertices[data.Indices.at(j + 2)], data.Vertices[data.Indices.at(j)]));
					}
					// Left
					tmpIndices.emplace_back(data.Indices.at(j));
					tmpIndices.emplace_back(i1->second);
					tmpIndices.emplace_back(i3->second);
					// Up
					tmpIndices.emplace_back(i1->second);
					tmpIndices.emplace_back(data.Indices.at(j + 1));
					tmpIndices.emplace_back(i2->second);
					// Right
					tmpIndices.emplace_back(i3->second);
					tmpIndices.emplace_back(i2->second);
					tmpIndices.emplace_back(data.Indices.at(j + 2));
					// Down
					tmpIndices.emplace_back(i1->second);
					tmpIndices.emplace_back(i2->second);
					tmpIndices.emplace_back(i3->second);
				}
				data.Indices = std::move(tmpIndices);
			}
			return data;
		}

		Data<Vertex> MakeUV(U32 latitudeDensity, U32 longitudeDensity) noexcept
		{
			if (!latitudeDensity)
				latitudeDensity = 1;
			if (!longitudeDensity)
				longitudeDensity = 1;
			latitudeDensity *= 3;
			longitudeDensity *= 3;

			const float latitudeAngle = Math::PI / Utils::SafeCast<float>(latitudeDensity);
			const float longitudeAngle = Math::PI2 / Utils::SafeCast<float>(longitudeDensity);
			const Vector base = Math::XMVectorSet(0.0f, RADIUS, 0.0f, 0.0f);

			Data<Vertex> data;
			data.Vertices.reserve(Utils::SafeCast<U64>(latitudeDensity - 1) * longitudeDensity * 4 + 2 * Utils::SafeCast<U64>(longitudeDensity));

			// Sphere vertices without poles
			for (U32 lat = 1, i = 0; lat < latitudeDensity; ++lat)
			{
				const auto latBase = Math::XMVector3Transform(base, Math::XMMatrixRotationX(latitudeAngle * static_cast<float>(lat)));
				for (U32 lon = 0; lon < longitudeDensity; ++lon)
				{
					const Vector vec = Math::XMVector3Transform(latBase, Math::XMMatrixRotationY(longitudeAngle * static_cast<float>(lon)));
					for (U8 j = 0; j < 4; ++j, ++i)
						Math::XMStoreFloat3(&data.Vertices.at(i).Position, vec);
				}
			}
			const auto getIndex = [&longitudeDensity](U32 lat, U32 lon) constexpr -> U32
				{
					return lat * (longitudeDensity << 2) + (lon << 2);
				};

			for (U32 lat = 0; lat < latitudeDensity - 2; ++lat)
			{
				for (U32 lon = 0; lon < longitudeDensity - 1; ++lon)
				{
					// Ring of rectangles around without last one
					U32 leftDown = getIndex(lat, lon);
					U32 rightUp = getIndex(lat + 1, lon + 1) + 2;
					data.Indices.emplace_back(leftDown);
					data.Indices.emplace_back(getIndex(lat + 1, lon) + 1);
					data.Indices.emplace_back(rightUp);
					data.Indices.emplace_back(leftDown);
					data.Indices.emplace_back(rightUp);
					data.Indices.emplace_back(getIndex(lat, lon + 1) + 3);
				}
				// Last rectangle to connect ring
				U32 leftDown = getIndex(lat, longitudeDensity - 1);
				U32 rightUp = getIndex(lat + 1, 0) + 2;
				data.Indices.emplace_back(leftDown);
				data.Indices.emplace_back(getIndex(lat + 1, longitudeDensity - 1) + 1);
				data.Indices.emplace_back(rightUp);
				data.Indices.emplace_back(leftDown);
				data.Indices.emplace_back(rightUp);
				data.Indices.emplace_back(getIndex(lat, 0) + 3);
			}

			// Poles vertices
			const U32 north = Utils::SafeCast<U32>(data.Vertices.size() - 2 * static_cast<U64>(longitudeDensity));
			for (U32 i = 0; i < longitudeDensity; ++i)
				Math::XMStoreFloat3(&data.Vertices.at(north + i).Position, Math::XMVectorNegate(base));

			const U32 south = north + longitudeDensity;
			for (U32 i = 0; i < longitudeDensity; ++i)
				Math::XMStoreFloat3(&data.Vertices.at(south + i).Position, base);

			// Triangle ring on each pole
			U32 leftDown = getIndex(latitudeDensity - 2, 0);
			U32 rightUp = getIndex(0, 1) + 2;
			for (U32 lon = 0; lon < longitudeDensity - 1; ++lon, rightUp += 4)
			{
				// North
				data.Indices.emplace_back(leftDown);
				data.Indices.emplace_back(north + lon);
				leftDown += 4;
				data.Indices.emplace_back(leftDown + 3);
				// South
				data.Indices.emplace_back(south + lon);
				data.Indices.emplace_back(rightUp - 5);
				data.Indices.emplace_back(rightUp);
			}
			// Last triangle to connect ring
			// North
			data.Indices.emplace_back(leftDown);
			data.Indices.emplace_back(south - 1);
			data.Indices.emplace_back(getIndex(latitudeDensity - 2, 0) + 3);
			// South
			data.Indices.emplace_back(Utils::SafeCast<U32>(data.Vertices.size() - 1));
			data.Indices.emplace_back(rightUp - 5);
			data.Indices.emplace_back(2);

			for (Vertex& v : data.Vertices)
			{
				// Generate normals
				Math::XMStoreFloat3(&v.Normal, Math::XMVector3Normalize(Math::XMLoadFloat3(&v.Position)));

				// UV mapping
				v.UV.x = std::atan2f(v.Position.z, v.Position.x) / Math::PI2 + 0.5f;
				v.UV.y = std::asinf(v.Position.y) / Math::PI + 0.5f;
			}

			ComputeTangents(data.Vertices, data.Indices);
			return data;
		}

		Data<Vertex> MakeIco(U32 density) noexcept
		{
			Data<Vertex> data
			{
				{
					// 0
					{ { 0.0f, RADIUS, 0.0f } },
					{ { 0.0f, RADIUS, 0.0f } },
					{ { 0.0f, RADIUS, 0.0f } },
					{ { 0.0f, RADIUS, 0.0f } },
					{ { 0.0f, RADIUS, 0.0f } },
					// 1
					{ { 0.0f, -RADIUS, 0.0f } },
					{ { 0.0f, -RADIUS, 0.0f } },
					{ { 0.0f, -RADIUS, 0.0f } },
					{ { 0.0f, -RADIUS, 0.0f } },
					{ { 0.0f, -RADIUS, 0.0f } },
					// 2
					{ { 0.0f, ICO_LEVEL, -ICO_CENTER_Z } },
					{ { 0.0f, ICO_LEVEL, -ICO_CENTER_Z } },
					{ { 0.0f, ICO_LEVEL, -ICO_CENTER_Z } },
					{ { 0.0f, ICO_LEVEL, -ICO_CENTER_Z } },
					{ { 0.0f, ICO_LEVEL, -ICO_CENTER_Z } },
					// 3
					{ { -ICO_BIG_X, ICO_LEVEL, -ICO_BIG_Z } },
					{ { -ICO_BIG_X, ICO_LEVEL, -ICO_BIG_Z } },
					{ { -ICO_BIG_X, ICO_LEVEL, -ICO_BIG_Z } },
					{ { -ICO_BIG_X, ICO_LEVEL, -ICO_BIG_Z } },
					{ { -ICO_BIG_X, ICO_LEVEL, -ICO_BIG_Z } },
					// 4
					{ { -ICO_SMALL_X, ICO_LEVEL, ICO_SMALL_Z } },
					{ { -ICO_SMALL_X, ICO_LEVEL, ICO_SMALL_Z } },
					{ { -ICO_SMALL_X, ICO_LEVEL, ICO_SMALL_Z } },
					{ { -ICO_SMALL_X, ICO_LEVEL, ICO_SMALL_Z } },
					{ { -ICO_SMALL_X, ICO_LEVEL, ICO_SMALL_Z } },
					// 5
					{ { ICO_SMALL_X, ICO_LEVEL, ICO_SMALL_Z } },
					{ { ICO_SMALL_X, ICO_LEVEL, ICO_SMALL_Z } },
					{ { ICO_SMALL_X, ICO_LEVEL, ICO_SMALL_Z } },
					{ { ICO_SMALL_X, ICO_LEVEL, ICO_SMALL_Z } },
					{ { ICO_SMALL_X, ICO_LEVEL, ICO_SMALL_Z } },
					// 6
					{ { ICO_BIG_X, ICO_LEVEL, -ICO_BIG_Z } },
					{ { ICO_BIG_X, ICO_LEVEL, -ICO_BIG_Z } },
					{ { ICO_BIG_X, ICO_LEVEL, -ICO_BIG_Z } },
					{ { ICO_BIG_X, ICO_LEVEL, -ICO_BIG_Z } },
					{ { ICO_BIG_X, ICO_LEVEL, -ICO_BIG_Z } },
					// 7
					{ { 0.0f, -ICO_LEVEL, ICO_CENTER_Z } },
					{ { 0.0f, -ICO_LEVEL, ICO_CENTER_Z } },
					{ { 0.0f, -ICO_LEVEL, ICO_CENTER_Z } },
					{ { 0.0f, -ICO_LEVEL, ICO_CENTER_Z } },
					{ { 0.0f, -ICO_LEVEL, ICO_CENTER_Z } },
					// 8
					{ { ICO_BIG_X, -ICO_LEVEL, ICO_BIG_Z } },
					{ { ICO_BIG_X, -ICO_LEVEL, ICO_BIG_Z } },
					{ { ICO_BIG_X, -ICO_LEVEL, ICO_BIG_Z } },
					{ { ICO_BIG_X, -ICO_LEVEL, ICO_BIG_Z } },
					{ { ICO_BIG_X, -ICO_LEVEL, ICO_BIG_Z } },
					// 9
					{ { ICO_SMALL_X, -ICO_LEVEL, -ICO_SMALL_Z } },
					{ { ICO_SMALL_X, -ICO_LEVEL, -ICO_SMALL_Z } },
					{ { ICO_SMALL_X, -ICO_LEVEL, -ICO_SMALL_Z } },
					{ { ICO_SMALL_X, -ICO_LEVEL, -ICO_SMALL_Z } },
					{ { ICO_SMALL_X, -ICO_LEVEL, -ICO_SMALL_Z } },
					// 10
					{ { -ICO_SMALL_X, -ICO_LEVEL, -ICO_SMALL_Z } },
					{ { -ICO_SMALL_X, -ICO_LEVEL, -ICO_SMALL_Z } },
					{ { -ICO_SMALL_X, -ICO_LEVEL, -ICO_SMALL_Z } },
					{ { -ICO_SMALL_X, -ICO_LEVEL, -ICO_SMALL_Z } },
					{ { -ICO_SMALL_X, -ICO_LEVEL, -ICO_SMALL_Z } },
					// 11
					{ { -ICO_BIG_X, -ICO_LEVEL, ICO_BIG_Z } },
					{ { -ICO_BIG_X, -ICO_LEVEL, ICO_BIG_Z } },
					{ { -ICO_BIG_X, -ICO_LEVEL, ICO_BIG_Z } },
					{ { -ICO_BIG_X, -ICO_LEVEL, ICO_BIG_Z } },
					{ { -ICO_BIG_X, -ICO_LEVEL, ICO_BIG_Z } }
				},
				{
					// Triangle ring
					17, 14, 53,
					59, 18, 52,
					22, 19, 58,
					39, 23, 57,
					27, 24, 38,
					44, 28, 37,
					32, 29, 43,
					49, 33, 42,
					12, 34, 48,
					54, 13, 47,
					// North
					15, 0, 11,
					20, 1, 16,
					25, 2, 21,
					30, 3, 26,
					10, 4, 31,
					// South
					36, 5, 40,
					41, 6, 45,
					46, 7, 50,
					51, 8, 55,
					56, 9, 35
				}
			};

			for (U32 i = 0; i < density; ++i)
			{
				std::vector<U32> tmpIndices;
				for (size_t j = 0; j < data.Indices.size(); j += 3)
				{
					U32 i0 = data.Indices.at(j);
					U32 i1 = data.Indices.at(j + 1);
					U32 i2 = data.Indices.at(j + 2);

					const Vertex& v0 = data.Vertices.at(i0);
					const Vertex& v1 = data.Vertices.at(i1);
					const Vertex& v2 = data.Vertices.at(i2);

					const Float3 left = Math::AddNormal(v0.Position, v1.Position);
					const Float3 right = Math::AddNormal(v1.Position, v2.Position);
					const Float3 down = Math::AddNormal(v2.Position, v0.Position);

					const U32 id = static_cast<U32>(data.Vertices.size());

					data.Vertices.emplace_back(left);  // 0
					data.Vertices.emplace_back(left);  // 1
					data.Vertices.emplace_back(left);  // 2
					data.Vertices.emplace_back(right); // 3
					data.Vertices.emplace_back(right); // 4
					data.Vertices.emplace_back(right); // 5
					data.Vertices.emplace_back(down);  // 6
					data.Vertices.emplace_back(down);  // 7
					data.Vertices.emplace_back(down);  // 8

					// Left
					tmpIndices.emplace_back(i0);
					tmpIndices.emplace_back(id + 2);
					tmpIndices.emplace_back(id + 6);
					// Up
					tmpIndices.emplace_back(id);
					tmpIndices.emplace_back(i1);
					tmpIndices.emplace_back(id + 5);
					// Right
					tmpIndices.emplace_back(id + 8);
					tmpIndices.emplace_back(id + 3);
					tmpIndices.emplace_back(i2);
					// Down
					tmpIndices.emplace_back(id + 1);
					tmpIndices.emplace_back(id + 4);
					tmpIndices.emplace_back(id + 7);
				}
				data.Indices = std::move(tmpIndices);
			}

			for (Vertex& v : data.Vertices)
			{
				// Generate normals
				Math::XMStoreFloat3(&v.Normal, Math::XMVector3Normalize(Math::XMLoadFloat3(&v.Position)));

				// Naive UV mapping first
				v.UV.x = std::atan2f(v.Position.z, v.Position.x) / Math::PI2 + 0.5f;
				v.UV.y = std::asinf(v.Position.y) / Math::PI + 0.5f;
			}

			// https://observablehq.com/@mourner/uv-mapping-an-icosphere
			// What if UV are outside 0-1 range? They should be wrapped in the shader
			for (U64 i = 0; i < data.Indices.size(); i += 3)
			{
				Float2& uv0 = data.Vertices.at(data.Indices.at(i)).UV;
				Float2& uv1 = data.Vertices.at(data.Indices.at(i + 1)).UV;
				Float2& uv2 = data.Vertices.at(data.Indices.at(i + 2)).UV;

				if (uv1.x - uv0.x >= 0.5f && uv0.y != 1.0f)
					uv1.x -= 1.0f;
				if (uv2.x - uv1.x > 0.5f)
					uv2.x -= 1.0f;
				if (uv0.x > 0.5f && uv0.x - uv2.x > 0.5f || uv0.x == 1.0f && uv2.y == 0.0f)
					uv0.x -= 1.0f;
				if (uv1.x > 0.5f && uv1.x - uv0.x > 0.5f)
					uv1.x -= 1.0f;

				if (uv0.y == 0.0f || uv0.y == 1.0f)
					uv0.x = (uv1.x + uv2.x) * 0.5f;
				if (uv1.y == 0.0f || uv1.y == 1.0f)
					uv1.x = (uv0.x + uv2.x) * 0.5f;
				if (uv2.y == 0.0f || uv2.y == 1.0f)
					uv2.x = (uv0.x + uv1.x) * 0.5f;
			}

			ComputeTangents(data.Vertices, data.Indices);
			return data;
		}
	}
}