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
			return h1 ^ (static_cast<size_t>(h2) << 1);
		}
	};
}

namespace ZE::GFX::Primitive
{
#pragma region Geometry computation
	template<bool GetSurfaceNormal, bool GetTangent>
	constexpr void ComputeGeometryData(std::vector<Vertex>& vertices, const std::vector<U32>& indices) noexcept
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
				ZE_ASSERT(Math::XMVectorGetX(Math::XMVector3Length(Math::XMLoadFloat3(&v0.Normal))) == 1.0f &&
					Math::XMVectorGetX(Math::XMVector3Length(Math::XMLoadFloat3(&v1.Normal))) == 1.0f &&
					Math::XMVectorGetX(Math::XMVector3Length(Math::XMLoadFloat3(&v2.Normal))) == 1.0f,
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
					Math::XMStoreFloat3(&v0.Tangent, Math::XMVector3Normalize(tangent));
					Math::XMStoreFloat3(&v1.Tangent, Math::XMVector3Normalize(tangent));
					Math::XMStoreFloat3(&v2.Tangent, Math::XMVector3Normalize(tangent));
				}
				else
				{
					normal = Math::XMLoadFloat3(&v0.Normal);
					Math::XMStoreFloat3(&v0.Tangent,
						Math::XMVector3Normalize(Math::XMVectorSubtract(tangent,
							Math::XMVectorMultiply(normal, Math::XMVector3Dot(tangent, normal)))));

					normal = Math::XMLoadFloat3(&v1.Normal);
					Math::XMStoreFloat3(&v1.Tangent,
						Math::XMVector3Normalize(Math::XMVectorSubtract(tangent,
							Math::XMVectorMultiply(normal, Math::XMVector3Dot(tangent, normal)))));

					normal = Math::XMLoadFloat3(&v2.Normal);
					Math::XMStoreFloat3(&v2.Tangent,
						Math::XMVector3Normalize(Math::XMVectorSubtract(tangent,
							Math::XMVectorMultiply(normal, Math::XMVector3Dot(tangent, normal)))));
				}

				// Store handness
				//v0.Tangent.w = direction;
				//v1.Tangent.w = direction;
				//v2.Tangent.w = direction;
			}
		}
	}

	void ComputeSurfaceNormalsTangents(std::vector<Vertex>& vertices, const std::vector<U32>& indices) noexcept
	{
		ComputeGeometryData<true, true>(vertices, indices);
	}

	void ComputeTangents(std::vector<Vertex>& vertices, const std::vector<U32>& indices) noexcept
	{
		ComputeGeometryData<false, true>(vertices, indices);
	}
#pragma endregion

#pragma region Cube
	std::vector<Float3> MakeCubeSolidVertex() noexcept
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

	std::vector<U32> MakeCubeSolidIndex() noexcept
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

	std::vector<U32> MakeCubeSolidIndexInverted() noexcept
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

	std::vector<Vertex> MakeCubeVertex(const std::vector<U32>& indices) noexcept
	{
		ZE_ASSERT(indices.size() % 3 == 0 && indices.size() > 2, "Incorrect index data!");
		constexpr float POINT = 0.5f;
		constexpr float UV_SHORT_1 = 1.0f / 3.0f;
		constexpr float UV_SHORT_2 = 2.0f / 3.0f;

		/*
		* UV mapping according to:
		* 0,0        ______             1,0
		*    _______| Left |____________
		*   | Front | Down | Back | Top |
		*    -------| Right|------------
		* 0,1        ------             1,1
		*/
		std::vector<Vertex> vertices
		{
			// Front
			{ { -POINT, -POINT, -POINT }, {}, { 0.25f, UV_SHORT_1 } },
			{ { -POINT, POINT, -POINT }, {}, { 0.0f, UV_SHORT_1 } },
			{ { POINT, POINT, -POINT }, {}, { 0.0f, UV_SHORT_2 } },
			{ { POINT, -POINT, -POINT }, {}, { 0.25f, UV_SHORT_2 } },
			// Left
			{ { -POINT, -POINT, POINT }, {}, { 0.5f, UV_SHORT_1 } },
			{ { -POINT, POINT, POINT }, {}, { 0.5f, 0.0f } },
			{ { -POINT, POINT, -POINT }, {}, { 0.25f, 0.0f } },
			{ { -POINT, -POINT, -POINT }, {}, { 0.25f, UV_SHORT_1 } },
			// Back
			{ { POINT, -POINT, POINT }, {}, { 0.5f, UV_SHORT_2 } },
			{ { POINT, POINT, POINT }, {}, { 0.75f, UV_SHORT_2 } },
			{ { -POINT, POINT, POINT }, {}, { 0.75f, UV_SHORT_1 } },
			{ { -POINT, -POINT, POINT }, {}, { 0.5f, UV_SHORT_1 } },
			// Right
			{ { POINT, -POINT, -POINT }, {}, { 0.25f, UV_SHORT_2 } },
			{ { POINT, POINT, -POINT }, {}, { 0.25f, 1.0f } },
			{ { POINT, POINT, POINT }, {}, { 0.5f, 1.0f } },
			{ { POINT, -POINT, POINT }, {}, { 0.5f, UV_SHORT_2 } },
			// Top
			{ { -POINT, POINT, -POINT }, {}, { 1.0f, UV_SHORT_1 } },
			{ { -POINT, POINT, POINT }, {}, { 0.75f, UV_SHORT_1 } },
			{ { POINT, POINT, POINT }, {}, { 0.75f, UV_SHORT_2 } },
			{ { POINT, POINT, -POINT }, {}, { 1.0f, UV_SHORT_2 } },
			// Down
			{ { -POINT, -POINT, POINT }, {}, { 0.5f, UV_SHORT_1 } },
			{ { -POINT, -POINT, -POINT }, {}, { 0.25f, UV_SHORT_1 } },
			{ { POINT, -POINT, -POINT }, {}, { 0.25f, UV_SHORT_2 } },
			{ { POINT, -POINT, POINT }, {}, { 0.5f, UV_SHORT_2 } },
		};
		ComputeSurfaceNormalsTangents(vertices, indices);
		return vertices;
	}

	std::vector<U32> MakeCubeIndex() noexcept
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
#pragma endregion

	Data<Float3> MakeConeSolid(U32 density) noexcept
	{
		if (!density)
			density = 1;
		density *= 3;

		Data<Float3> data;
		data.Vertices.resize(static_cast<U64>(density) + 1);

		data.Vertices.at(0) = { 0.0f, 1.0f, 0.0f };
		data.Vertices.at(1) = { 0.0f, 0.0f, 1.0f }; // Base of circle

		const float angle = 2.0f * static_cast<float>(M_PI / density);
		const Vector base = Math::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

		for (U64 i = 1; i < density; ++i)
		{
			Math::XMStoreFloat3(&data.Vertices.at(i + 1),
				Math::XMVector3Transform(base, Math::XMMatrixRotationY(angle * static_cast<float>(i))));
		}

		data.Indices.reserve(static_cast<U64>(density - 1) * 6);
		for (U32 i = 2; i < density; ++i)
		{
			// Cone wall
			data.Indices.emplace_back(0U);
			data.Indices.emplace_back(i);
			data.Indices.emplace_back(i + 1);
			// Cone base
			data.Indices.emplace_back(1U);
			data.Indices.emplace_back(i + 1);
			data.Indices.emplace_back(i);
		}
		// First wall
		data.Indices.emplace_back(0U);
		data.Indices.emplace_back(1U);
		data.Indices.emplace_back(2U);
		// Last wall
		data.Indices.emplace_back(0U);
		data.Indices.emplace_back(density);
		data.Indices.emplace_back(1U);

		return data;
	}

	Data<Float3> MakeSphereIcoSolid(U32 density) noexcept
	{
		constexpr float BIG_ANGLE = static_cast<float>(M_PI / 10.0f);
		constexpr float SMALL_ANGLE = static_cast<float>(M_PI * 0.3f);
		const float baseAngle = atanf(0.5f);

		const float centerZ = cosf(baseAngle);
		const float bigX = centerZ * cosf(BIG_ANGLE);
		const float bigZ = centerZ * sinf(BIG_ANGLE);
		const float smallX = centerZ * cosf(SMALL_ANGLE);
		const float smallZ = centerZ * sinf(SMALL_ANGLE);
		const float level = sinf(baseAngle);

		Data<Float3> data
		{
			{
				{ 0.0f, 1.0f, 0.0f },
				{ 0.0f, -1.0f, 0.0f },
				{ 0.0f, level, -centerZ },
				{ -bigX, level, -bigZ },
				{ -smallX, level, smallZ },
				{ smallX, level, smallZ },
				{ bigX, level, -bigZ },
				{ 0.0f, -level, centerZ },
				{ bigX, -level, bigZ },
				{ smallX, -level, -smallZ },
				{ -smallX, -level, -smallZ },
				{ -bigX, -level, bigZ }
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
					i1 = lookup.emplace(std::make_pair(data.Indices.at(j), data.Indices.at(j + 1)), static_cast<U32>(data.Vertices.size())).first;
					data.Vertices.emplace_back(Math::AddNormal(data.Vertices[data.Indices.at(j)], data.Vertices[data.Indices.at(j + 1)]));
				}
				auto i2 = lookup.find({ data.Indices.at(j + 1), data.Indices.at(j + 2) });
				if (i2 == lookup.end())
				{
					i2 = lookup.emplace(std::make_pair(data.Indices.at(j + 1), data.Indices.at(j + 2)), static_cast<U32>(data.Vertices.size())).first;
					data.Vertices.emplace_back(Math::AddNormal(data.Vertices[data.Indices.at(j + 1)], data.Vertices[data.Indices.at(j + 2)]));
				}
				auto i3 = lookup.find({ data.Indices.at(j + 2), data.Indices.at(j) });
				if (i3 == lookup.end())
				{
					i3 = lookup.emplace(std::make_pair(data.Indices.at(j + 2), data.Indices.at(j)), static_cast<U32>(data.Vertices.size())).first;
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
}