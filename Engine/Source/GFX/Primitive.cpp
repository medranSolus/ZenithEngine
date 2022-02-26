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
	std::vector<Float3> MakeCubeVertex() noexcept
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

	std::vector<U32> MakeCubeIndex() noexcept
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

	std::vector<U32> MakeCubeIndexInverted() noexcept
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

	std::pair<std::vector<Float3>, std::vector<U32>> MakeSphereIco(U32 density) noexcept
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

		std::vector<Float3> vertices
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
		};

		std::vector<U32> indices
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
		};

		auto comparator = [](const Key& x, const Key& y) constexpr -> bool
		{
			return (x.first == y.first && x.second == y.second) || (x.first == y.second && x.second == y.first);
		};
		for (U32 i = 0; i < density; ++i)
		{
			std::unordered_map<Key, U32, std::hash<Key>, decltype(comparator)> lookup(0, std::hash<Key>{}, comparator);
			std::vector<U32> tmpIndices;
			for (U64 j = 0; j < indices.size(); j += 3)
			{
				auto i1 = lookup.find({ indices.at(j), indices.at(j + 1) });
				if (i1 == lookup.end())
				{
					i1 = lookup.emplace(std::make_pair(indices.at(j), indices.at(j + 1)), static_cast<U32>(vertices.size())).first;
					vertices.emplace_back(Math::AddNormal(vertices[indices.at(j)], vertices[indices.at(j + 1)]));
				}
				auto i2 = lookup.find({ indices.at(j + 1), indices.at(j + 2) });
				if (i2 == lookup.end())
				{
					i2 = lookup.emplace(std::make_pair(indices.at(j + 1), indices.at(j + 2)), static_cast<U32>(vertices.size())).first;
					vertices.emplace_back(Math::AddNormal(vertices[indices.at(j + 1)], vertices[indices.at(j + 2)]));
				}
				auto i3 = lookup.find({ indices.at(j + 2), indices.at(j) });
				if (i3 == lookup.end())
				{
					i3 = lookup.emplace(std::make_pair(indices.at(j + 2), indices.at(j)), static_cast<U32>(vertices.size())).first;
					vertices.emplace_back(Math::AddNormal(vertices[indices.at(j + 2)], vertices[indices.at(j)]));
				}
				// Left
				tmpIndices.emplace_back(indices.at(j));
				tmpIndices.emplace_back(i1->second);
				tmpIndices.emplace_back(i3->second);
				// Up
				tmpIndices.emplace_back(i1->second);
				tmpIndices.emplace_back(indices.at(j + 1));
				tmpIndices.emplace_back(i2->second);
				// Right
				tmpIndices.emplace_back(i3->second);
				tmpIndices.emplace_back(i2->second);
				tmpIndices.emplace_back(indices.at(j + 2));
				// Down
				tmpIndices.emplace_back(i1->second);
				tmpIndices.emplace_back(i2->second);
				tmpIndices.emplace_back(i3->second);
			}
			indices = std::move(tmpIndices);
		}
		return { std::move(vertices), std::move(indices) };
	}
}