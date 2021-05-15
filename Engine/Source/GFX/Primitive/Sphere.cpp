#include "GFX/Primitive/Sphere.h"
#include <unordered_map>

typedef std::pair<U32, U32> Key;
#pragma region Hasher for Ico Sphere Generation
namespace std
{
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
#pragma endregion

namespace ZE::GFX::Primitive
{
	std::string Sphere::GetNameUVSolid(U32 latitudeDensity, U32 longitudeDensity, const std::vector<VertexAttribute>& attributes) noexcept
	{
		std::string name = "U";
		for (const auto& attrib : attributes)
			name += std::to_string(static_cast<U8>(attrib));
		if (!latitudeDensity)
			latitudeDensity = 1;
		if (!longitudeDensity)
			longitudeDensity = 1;
		latitudeDensity *= 3;
		longitudeDensity *= 3;
		return name + std::to_string(latitudeDensity) + "S" + std::to_string(longitudeDensity);
	}

	std::string Sphere::GetNameUV(U32 latitudeDensity, U32 longitudeDensity, const std::vector<VertexAttribute>& attributes) noexcept
	{
		std::string name = "U";
		for (const auto& attrib : attributes)
			name += std::to_string(static_cast<U8>(attrib));
		if (!latitudeDensity)
			latitudeDensity = 1;
		if (!longitudeDensity)
			longitudeDensity = 1;
		latitudeDensity *= 3;
		longitudeDensity *= 3;
		return name + std::to_string(latitudeDensity) + "N" + std::to_string(longitudeDensity);
	}

	std::shared_ptr<Data::VertexLayout> Sphere::GetLayoutUVSolid(const std::vector<VertexAttribute>& attributes) noexcept
	{
		std::shared_ptr<Data::VertexLayout> layout = std::make_shared<Data::VertexLayout>();
		for (const auto& attrib : attributes)
			layout->Append(attrib);
		return layout;
	}

	std::shared_ptr<Data::VertexLayout> Sphere::GetLayoutUV(const std::vector<VertexAttribute>& attributes) noexcept
	{
		std::shared_ptr<Data::VertexLayout> layout = std::make_shared<Data::VertexLayout>();
		layout->Append(VertexAttribute::Normal);
		for (const auto& attrib : attributes)
			layout->Append(attrib);
		return layout;
	}

	IndexedTriangleList Sphere::MakeUVSolid(U32 latitudeDensity, U32 longitudeDensity, const std::vector<VertexAttribute>& attributes) noexcept
	{
		if (!latitudeDensity)
			latitudeDensity = 1;
		if (!longitudeDensity)
			longitudeDensity = 1;
		latitudeDensity *= 3;
		longitudeDensity *= 3;
		constexpr float RADIUS = 1.0f;
		const float latitudeAngle = static_cast<float>(M_PI / latitudeDensity);
		const float longitudeAngle = 2.0f * static_cast<float>(M_PI / longitudeDensity);
		Vector base = Math::XMVectorSet(0.0f, RADIUS, 0.0f, 0.0f);

		Data::VertexBufferData vertices(GetLayoutUVSolid(attributes),
			static_cast<U64>(latitudeDensity - 1) * longitudeDensity + 2);
		vertices.SetBox({ RADIUS, -RADIUS, -RADIUS, RADIUS, -RADIUS, RADIUS });

		// Sphere vertices without poles
		for (U32 lat = 1, i = 0; lat < latitudeDensity; ++lat)
		{
			const auto latBase = Math::XMVector3Transform(base, Math::XMMatrixRotationX(latitudeAngle * lat));
			for (U32 lon = 0; lon < longitudeDensity; ++lon, ++i)
			{
				Math::XMStoreFloat3(&vertices[i].Get<VertexAttribute::Position3D>(),
					Math::XMVector3Transform(latBase, Math::XMMatrixRotationY(longitudeAngle * lon)));
			}
		}
		const auto getIndex = [&latitudeDensity, &longitudeDensity](U32 lat, U32 lon) constexpr -> U32
		{
			return lat * longitudeDensity + lon;
		};
		U32 baseIndex;
		std::vector<U32> indices;
		for (U32 lat = 0; lat < latitudeDensity - 2; ++lat)
		{
			for (U32 lon = 0; lon < longitudeDensity - 1; ++lon)
			{
				// Ring of rectangles around without last one
				baseIndex = getIndex(lat, lon);
				indices.push_back(baseIndex);
				indices.push_back(baseIndex + longitudeDensity);
				indices.push_back(baseIndex + longitudeDensity + 1);
				indices.push_back(baseIndex);
				indices.push_back(baseIndex + longitudeDensity + 1);
				indices.push_back(baseIndex + 1);
			}
			// Last rectangle to connect ring
			baseIndex = getIndex(lat, 0);
			indices.push_back(baseIndex);
			indices.push_back(baseIndex + longitudeDensity - 1);
			indices.push_back(baseIndex + 2 * longitudeDensity - 1);
			indices.push_back(baseIndex);
			indices.push_back(baseIndex + 2 * longitudeDensity - 1);
			indices.push_back(baseIndex + longitudeDensity);
		}
		// Poles vertices
		const U32 pole = static_cast<U32>(vertices.Size() - 2);
		// South
		Math::XMStoreFloat3(&vertices[pole].Get<VertexAttribute::Position3D>(), Math::XMVectorNegate(base));
		// North
		Math::XMStoreFloat3(&vertices[static_cast<U64>(pole) + 1].Get<VertexAttribute::Position3D>(), std::move(base));

		// Triangle ring on each pole
		for (U32 lon = 0; lon < longitudeDensity - 1; ++lon)
		{
			// North
			baseIndex = getIndex(0, lon);
			indices.push_back(pole + 1);
			indices.push_back(baseIndex);
			indices.push_back(baseIndex + 1);
			// South
			baseIndex = getIndex(latitudeDensity - 2, lon);
			indices.push_back(baseIndex + 1);
			indices.push_back(baseIndex);
			indices.push_back(pole);
		}
		// Last triangle to connect ring
		// North
		indices.push_back(pole + 1);
		indices.push_back(longitudeDensity - 1);
		indices.push_back(0);
		// South
		baseIndex = getIndex(latitudeDensity - 2, 0);
		indices.push_back(baseIndex);
		indices.push_back(baseIndex + longitudeDensity - 1);
		indices.push_back(pole);
		return { std::move(vertices), std::move(indices) };
	}

	IndexedTriangleList Sphere::MakeUV(U32 latitudeDensity, U32 longitudeDensity, const std::vector<VertexAttribute>& attributes) noexcept
	{
		if (!latitudeDensity)
			latitudeDensity = 1;
		if (!longitudeDensity)
			longitudeDensity = 1;
		latitudeDensity *= 3;
		longitudeDensity *= 3;
		constexpr float RADIUS = 1.0f;
		const float latitudeAngle = static_cast<float>(M_PI / latitudeDensity);
		const float longitudeAngle = 2.0f * static_cast<float>(M_PI / longitudeDensity);
		Vector base = Math::XMVectorSet(0.0f, RADIUS, 0.0f, 0.0f);

		Data::VertexBufferData vertices(GetLayoutUV(attributes),
			static_cast<U64>(latitudeDensity - 1) * longitudeDensity * 4 + 2 * static_cast<U64>(longitudeDensity));
		vertices.SetBox({ RADIUS, -RADIUS, -RADIUS, RADIUS, -RADIUS, RADIUS });

		// Sphere vertices without poles
		for (U32 lat = 1, i = 0; lat < latitudeDensity; ++lat)
		{
			const auto latBase = Math::XMVector3Transform(base, Math::XMMatrixRotationX(latitudeAngle * lat));
			for (U32 lon = 0; lon < longitudeDensity; ++lon)
			{
				const Vector vec = Math::XMVector3Transform(latBase, Math::XMMatrixRotationY(longitudeAngle * lon));
				for (U8 j = 0; j < 4; ++j, ++i)
					Math::XMStoreFloat3(&vertices[i].Get<VertexAttribute::Position3D>(), vec);
			}
		}
		const auto getIndex = [&latitudeDensity, &longitudeDensity](U32 lat, U32 lon) constexpr -> U32
		{
			return lat * (longitudeDensity << 2) + (lon << 2);
		};
		U32 leftDown, rightUp;
		std::vector<U32> indices;
		for (U32 lat = 0; lat < latitudeDensity - 2; ++lat)
		{
			for (U32 lon = 0; lon < longitudeDensity - 1; ++lon)
			{
				// Ring of rectangles around without last one
				leftDown = getIndex(lat, lon);
				rightUp = getIndex(lat + 1, lon + 1) + 2;
				indices.push_back(leftDown);
				indices.push_back(getIndex(lat + 1, lon) + 1);
				indices.push_back(rightUp);
				indices.push_back(leftDown);
				indices.push_back(rightUp);
				indices.push_back(getIndex(lat, lon + 1) + 3);
			}
			// Last rectangle to connect ring
			leftDown = getIndex(lat, longitudeDensity - 1);
			rightUp = getIndex(lat + 1, 0) + 2;
			indices.push_back(leftDown);
			indices.push_back(getIndex(lat + 1, longitudeDensity - 1) + 1);
			indices.push_back(rightUp);
			indices.push_back(leftDown);
			indices.push_back(rightUp);
			indices.push_back(getIndex(lat, 0) + 3);
		}
		// Poles vertices
		const U32 north = static_cast<U32>(vertices.Size() - 2 * static_cast<U64>(longitudeDensity));
		for (U8 i = 0; i < longitudeDensity; ++i)
			Math::XMStoreFloat3(&vertices[static_cast<U64>(north) + i].Get<VertexAttribute::Position3D>(), Math::XMVectorNegate(base));

		const U32 south = static_cast<U32>(north + longitudeDensity);
		for (U8 i = 0; i < longitudeDensity; ++i)
			Math::XMStoreFloat3(&vertices[static_cast<U64>(south) + i].Get<VertexAttribute::Position3D>(), base);

		// Triangle ring on each pole
		leftDown = getIndex(latitudeDensity - 2, 0);
		rightUp = getIndex(0, 1) + 2;
		for (U32 lon = 0; lon < longitudeDensity - 1; ++lon, rightUp += 4)
		{
			// North
			indices.push_back(leftDown);
			indices.push_back(north + lon);
			leftDown += 4;
			indices.push_back(leftDown + 3);
			// South
			indices.push_back(south + lon);
			indices.push_back(rightUp - 5);
			indices.push_back(rightUp);
		}
		// Last triangle to connect ring
		// North
		indices.push_back(leftDown);
		indices.push_back(south - 1);
		indices.push_back(getIndex(latitudeDensity - 2, 0) + 3);
		// South
		indices.push_back(static_cast<U32>(vertices.Size() - 1));
		indices.push_back(rightUp - 5);
		indices.push_back(2);

		Float3 normal;
		for (U64 i = 0; i < vertices.Size(); ++i)
		{
			Math::XMStoreFloat3(&normal, Math::XMVector3Normalize(Math::XMLoadFloat3(&vertices[i].Get<VertexAttribute::Position3D>())));
			vertices[i].SetByIndex(1, normal);
		}
		return { std::move(vertices), std::move(indices) };
	}

	std::string Sphere::GetNameIcoSolid(U32 density, const std::vector<VertexAttribute>& attributes) noexcept
	{
		std::string name = "I";
		for (const auto& attrib : attributes)
			name += std::to_string(static_cast<U8>(attrib));
		return name + "S" + std::to_string(density);
	}

	std::string Sphere::GetNameIco(U32 density, const std::vector<VertexAttribute>& attributes) noexcept
	{
		std::string name = "I";
		for (const auto& attrib : attributes)
			name += std::to_string(static_cast<U8>(attrib));
		return name + "N" + std::to_string(density);
	}

	IndexedTriangleList Sphere::MakeIcoSolid(U32 density, const std::vector<VertexAttribute>& attributes) noexcept
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

		Data::VertexBufferData vertices(GetLayoutIcoSolid(attributes), 12);
		vertices.SetBox({ 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f });

		vertices[0].SetByIndex(0, Float3(0.0f, 1.0f, 0.0f));
		vertices[1].SetByIndex(0, Float3(0.0f, -1.0f, 0.0f));
		vertices[2].SetByIndex(0, Float3(0.0f, level, -centerZ));
		vertices[3].SetByIndex(0, Float3(-bigX, level, -bigZ));
		vertices[4].SetByIndex(0, Float3(-smallX, level, smallZ));
		vertices[5].SetByIndex(0, Float3(smallX, level, smallZ));
		vertices[6].SetByIndex(0, Float3(bigX, level, -bigZ));
		vertices[7].SetByIndex(0, Float3(0.0f, -level, centerZ));
		vertices[8].SetByIndex(0, Float3(bigX, -level, bigZ));
		vertices[9].SetByIndex(0, Float3(smallX, -level, -smallZ));
		vertices[10].SetByIndex(0, Float3(-smallX, -level, -smallZ));
		vertices[11].SetByIndex(0, Float3(-bigX, -level, bigZ));

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
					i1 = lookup.emplace(std::make_pair(indices.at(j), indices.at(j + 1)), static_cast<U32>(vertices.Size())).first;
					vertices.EmplaceBack(Math::AddNormal(vertices[indices.at(j)].Get<VertexAttribute::Position3D>(),
						vertices[indices.at(j + 1)].Get<VertexAttribute::Position3D>()));
				}
				auto i2 = lookup.find({ indices.at(j + 1), indices.at(j + 2) });
				if (i2 == lookup.end())
				{
					i2 = lookup.emplace(std::make_pair(indices.at(j + 1), indices.at(j + 2)), static_cast<U32>(vertices.Size())).first;
					vertices.EmplaceBack(Math::AddNormal(vertices[indices.at(j + 1)].Get<VertexAttribute::Position3D>(),
						vertices[indices.at(j + 2)].Get<VertexAttribute::Position3D>()));
				}
				auto i3 = lookup.find({ indices.at(j + 2), indices.at(j) });
				if (i3 == lookup.end())
				{
					i3 = lookup.emplace(std::make_pair(indices.at(j + 2), indices.at(j)), static_cast<U32>(vertices.Size())).first;
					vertices.EmplaceBack(Math::AddNormal(vertices[indices.at(j + 2)].Get<VertexAttribute::Position3D>(),
						vertices[indices.at(j)].Get<VertexAttribute::Position3D>()));
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

	IndexedTriangleList Sphere::MakeIco(U32 density, const std::vector<VertexAttribute>& attributes) noexcept
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

		Data::VertexBufferData vertices(GetLayoutIco(attributes), 60);
		vertices.SetBox({ 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f });

		// 0
		vertices[0].SetByIndex(0, Float3(0.0f, 1.0f, 0.0f));
		vertices[1].SetByIndex(0, Float3(0.0f, 1.0f, 0.0f));
		vertices[2].SetByIndex(0, Float3(0.0f, 1.0f, 0.0f));
		vertices[3].SetByIndex(0, Float3(0.0f, 1.0f, 0.0f));
		vertices[4].SetByIndex(0, Float3(0.0f, 1.0f, 0.0f));
		// 1
		vertices[5].SetByIndex(0, Float3(0.0f, -1.0f, 0.0f));
		vertices[6].SetByIndex(0, Float3(0.0f, -1.0f, 0.0f));
		vertices[7].SetByIndex(0, Float3(0.0f, -1.0f, 0.0f));
		vertices[8].SetByIndex(0, Float3(0.0f, -1.0f, 0.0f));
		vertices[9].SetByIndex(0, Float3(0.0f, -1.0f, 0.0f));
		// 2
		vertices[10].SetByIndex(0, Float3(0.0f, level, -centerZ));
		vertices[11].SetByIndex(0, Float3(0.0f, level, -centerZ));
		vertices[12].SetByIndex(0, Float3(0.0f, level, -centerZ));
		vertices[13].SetByIndex(0, Float3(0.0f, level, -centerZ));
		vertices[14].SetByIndex(0, Float3(0.0f, level, -centerZ));
		// 3
		vertices[15].SetByIndex(0, Float3(-bigX, level, -bigZ));
		vertices[16].SetByIndex(0, Float3(-bigX, level, -bigZ));
		vertices[17].SetByIndex(0, Float3(-bigX, level, -bigZ));
		vertices[18].SetByIndex(0, Float3(-bigX, level, -bigZ));
		vertices[19].SetByIndex(0, Float3(-bigX, level, -bigZ));
		// 4
		vertices[20].SetByIndex(0, Float3(-smallX, level, smallZ));
		vertices[21].SetByIndex(0, Float3(-smallX, level, smallZ));
		vertices[22].SetByIndex(0, Float3(-smallX, level, smallZ));
		vertices[23].SetByIndex(0, Float3(-smallX, level, smallZ));
		vertices[24].SetByIndex(0, Float3(-smallX, level, smallZ));
		// 5
		vertices[25].SetByIndex(0, Float3(smallX, level, smallZ));
		vertices[26].SetByIndex(0, Float3(smallX, level, smallZ));
		vertices[27].SetByIndex(0, Float3(smallX, level, smallZ));
		vertices[28].SetByIndex(0, Float3(smallX, level, smallZ));
		vertices[29].SetByIndex(0, Float3(smallX, level, smallZ));
		// 6
		vertices[30].SetByIndex(0, Float3(bigX, level, -bigZ));
		vertices[31].SetByIndex(0, Float3(bigX, level, -bigZ));
		vertices[32].SetByIndex(0, Float3(bigX, level, -bigZ));
		vertices[33].SetByIndex(0, Float3(bigX, level, -bigZ));
		vertices[34].SetByIndex(0, Float3(bigX, level, -bigZ));
		// 7
		vertices[35].SetByIndex(0, Float3(0.0f, -level, centerZ));
		vertices[36].SetByIndex(0, Float3(0.0f, -level, centerZ));
		vertices[37].SetByIndex(0, Float3(0.0f, -level, centerZ));
		vertices[38].SetByIndex(0, Float3(0.0f, -level, centerZ));
		vertices[39].SetByIndex(0, Float3(0.0f, -level, centerZ));
		// 8
		vertices[40].SetByIndex(0, Float3(bigX, -level, bigZ));
		vertices[41].SetByIndex(0, Float3(bigX, -level, bigZ));
		vertices[42].SetByIndex(0, Float3(bigX, -level, bigZ));
		vertices[43].SetByIndex(0, Float3(bigX, -level, bigZ));
		vertices[44].SetByIndex(0, Float3(bigX, -level, bigZ));
		// 9
		vertices[45].SetByIndex(0, Float3(smallX, -level, -smallZ));
		vertices[46].SetByIndex(0, Float3(smallX, -level, -smallZ));
		vertices[47].SetByIndex(0, Float3(smallX, -level, -smallZ));
		vertices[48].SetByIndex(0, Float3(smallX, -level, -smallZ));
		vertices[49].SetByIndex(0, Float3(smallX, -level, -smallZ));
		// 10
		vertices[50].SetByIndex(0, Float3(-smallX, -level, -smallZ));
		vertices[51].SetByIndex(0, Float3(-smallX, -level, -smallZ));
		vertices[52].SetByIndex(0, Float3(-smallX, -level, -smallZ));
		vertices[53].SetByIndex(0, Float3(-smallX, -level, -smallZ));
		vertices[54].SetByIndex(0, Float3(-smallX, -level, -smallZ));
		// 11
		vertices[55].SetByIndex(0, Float3(-bigX, -level, bigZ));
		vertices[56].SetByIndex(0, Float3(-bigX, -level, bigZ));
		vertices[57].SetByIndex(0, Float3(-bigX, -level, bigZ));
		vertices[58].SetByIndex(0, Float3(-bigX, -level, bigZ));
		vertices[59].SetByIndex(0, Float3(-bigX, -level, bigZ));

		std::vector<U32> indices
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
		};

		for (U32 i = 0; i < density; ++i)
		{
			std::vector<U32> tmpIndices;
			for (size_t j = 0; j < indices.size(); j += 3)
			{
				Data::Vertex v0 = vertices[indices.at(j)];
				Data::Vertex v1 = vertices[indices.at(j + 1)];
				Data::Vertex v2 = vertices[indices.at(j + 2)];
				const Float3 left = Math::AddNormal(v0.Get<VertexAttribute::Position3D>(), v1.Get<VertexAttribute::Position3D>());
				const Float3 right = Math::AddNormal(v1.Get<VertexAttribute::Position3D>(), v2.Get<VertexAttribute::Position3D>());
				const Float3 down = Math::AddNormal(v2.Get<VertexAttribute::Position3D>(), v0.Get<VertexAttribute::Position3D>());
				const U32 id = static_cast<U32>(vertices.Size());
				vertices.EmplaceBack(left);  // 0
				vertices.EmplaceBack(left);  // 1
				vertices.EmplaceBack(left);  // 2
				vertices.EmplaceBack(right); // 3
				vertices.EmplaceBack(right); // 4
				vertices.EmplaceBack(right); // 5
				vertices.EmplaceBack(down);  // 6
				vertices.EmplaceBack(down);  // 7
				vertices.EmplaceBack(down);  // 8

				// Left
				tmpIndices.emplace_back(indices.at(j));
				tmpIndices.emplace_back(id + 2);
				tmpIndices.emplace_back(id + 6);
				// Up
				tmpIndices.emplace_back(id);
				tmpIndices.emplace_back(indices.at(j + 1));
				tmpIndices.emplace_back(id + 5);
				// Right
				tmpIndices.emplace_back(id + 8);
				tmpIndices.emplace_back(id + 3);
				tmpIndices.emplace_back(indices.at(j + 2));
				// Down
				tmpIndices.emplace_back(id + 1);
				tmpIndices.emplace_back(id + 4);
				tmpIndices.emplace_back(id + 7);
			}
			indices = std::move(tmpIndices);
		}
		Float3 normal;
		for (U64 i = 0; i < vertices.Size(); ++i)
		{
			Math::XMStoreFloat3(&normal, Math::XMVector3Normalize(Math::XMLoadFloat3(&vertices[i].Get<VertexAttribute::Position3D>())));
			vertices[i].SetByIndex(1, normal);
		}
		return { std::move(vertices), std::move(indices) };
	}
}