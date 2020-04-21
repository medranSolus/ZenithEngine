#include "Sphere.h"
#include "Math.h"
#include <boost/functional/hash/hash.hpp>
#include <unordered_map>

namespace GFX::Primitive
{
	IndexedTriangleList Sphere::MakeSolidUV(unsigned int latitudeDensity, unsigned int longitudeDensity, const std::vector<VertexAttribute>&& attributes)
	{
		if (!latitudeDensity)
			latitudeDensity = 1;
		if (!longitudeDensity)
			longitudeDensity = 1;
		latitudeDensity *= 3;
		longitudeDensity *= 3;
		constexpr float radius = 1.0f;
		const auto base = DirectX::XMVectorSet(0.0f, radius, 0.0f, 0.0f);
		const float latitudeAngle = static_cast<float>(M_PI / latitudeDensity);
		const float longitudeAngle = 2.0f * static_cast<float>(M_PI / longitudeDensity);

		std::shared_ptr<Data::VertexLayout> layout = std::make_shared<Data::VertexLayout>();
		for (const auto& attrib : attributes)
			layout->Append(attrib);
		Data::VertexBufferData vertices(layout, (latitudeDensity - 1) * longitudeDensity + 2);
		// Sphere vertices without poles
		for (unsigned int lat = 1, i = 0; lat < latitudeDensity; ++lat)
		{
			const auto latBase = DirectX::XMVector3Transform(base, DirectX::XMMatrixRotationX(latitudeAngle * lat));
			for (unsigned int lon = 0; lon < longitudeDensity; ++lon, ++i)
				DirectX::XMStoreFloat3(&vertices[i].Get<VertexAttribute::Position3D>(),
					std::move(DirectX::XMVector3Transform(latBase, DirectX::XMMatrixRotationY(longitudeAngle * lon))));
		}
		const auto getIndex = [&latitudeDensity, &longitudeDensity](unsigned int lat, unsigned int lon) constexpr -> unsigned int
		{
			return lat * longitudeDensity + lon;
		};
		unsigned int baseIndex;
		std::vector<unsigned int> indices;
		for (unsigned int lat = 0; lat < latitudeDensity - 2; ++lat)
		{
			for (unsigned int lon = 0; lon < longitudeDensity - 1; ++lon)
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
		const unsigned int pole = static_cast<unsigned int>(vertices.Size() - 2);
		// South
		DirectX::XMStoreFloat3(&vertices[pole].Get<VertexAttribute::Position3D>(), std::move(DirectX::XMVectorNegate(base)));
		// North
		DirectX::XMStoreFloat3(&vertices[pole + 1].Get<VertexAttribute::Position3D>(), std::move(base));

		// Triangle ring on each pole
		for (unsigned int lon = 0; lon < longitudeDensity - 1; ++lon)
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
		return
		{
			std::move(vertices), std::move(indices),
			std::string(typeid(Primitive::Sphere).name()) + "UVSla" + std::to_string(latitudeDensity) + "lo" + std::to_string(longitudeDensity)
		};
	}
	
	IndexedTriangleList Sphere::MakeUV(unsigned int latitudeDensity, unsigned int longitudeDensity, const std::vector<VertexAttribute>&& attributes)
	{
		if (!latitudeDensity)
			latitudeDensity = 1;
		if (!longitudeDensity)
			longitudeDensity = 1;
		latitudeDensity *= 3;
		longitudeDensity *= 3;
		constexpr float radius = 1.0f;
		const auto base = DirectX::XMVectorSet(0.0f, radius, 0.0f, 0.0f);
		const float latitudeAngle = static_cast<float>(M_PI / latitudeDensity);
		const float longitudeAngle = 2.0f * static_cast<float>(M_PI / longitudeDensity);

		std::shared_ptr<Data::VertexLayout> layout = std::make_shared<Data::VertexLayout>();
		layout->Append(VertexAttribute::Normal);
		for (const auto& attrib : attributes)
			layout->Append(attrib);
		Data::VertexBufferData vertices(layout, (latitudeDensity - 1) * longitudeDensity * 4 + 2 * longitudeDensity);
		// Sphere vertices without poles
		for (unsigned int lat = 1, i = 0; lat < latitudeDensity; ++lat)
		{
			const auto latBase = DirectX::XMVector3Transform(base, DirectX::XMMatrixRotationX(latitudeAngle * lat));
			for (unsigned int lon = 0; lon < longitudeDensity; ++lon)
			{
				const DirectX::XMVECTOR vec = DirectX::XMVector3Transform(latBase, DirectX::XMMatrixRotationY(longitudeAngle * lon));
				for (unsigned char j = 0; j < 4; ++j, ++i)
					DirectX::XMStoreFloat3(&vertices[i].Get<VertexAttribute::Position3D>(), vec);
			}
		}
		const auto getIndex = [&latitudeDensity, &longitudeDensity](unsigned int lat, unsigned int lon) constexpr -> unsigned int
		{
			return lat * (longitudeDensity << 2) + (lon << 2);
		};
		unsigned int leftDown, rightUp;
		std::vector<unsigned int> indices;
		for (unsigned int lat = 0; lat < latitudeDensity - 2; ++lat)
		{
			for (unsigned int lon = 0; lon < longitudeDensity - 1; ++lon)
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
		const unsigned int north = static_cast<unsigned int>(vertices.Size() - 2 * longitudeDensity);
		for (unsigned char i = 0; i < longitudeDensity; ++i)
			DirectX::XMStoreFloat3(&vertices[north + i].Get<VertexAttribute::Position3D>(), std::move(DirectX::XMVectorNegate(base)));

		const unsigned int south = static_cast<unsigned int>(north + longitudeDensity);
		for (unsigned char i = 0; i < longitudeDensity; ++i)
			DirectX::XMStoreFloat3(&vertices[south + i].Get<VertexAttribute::Position3D>(), std::move(base));

		// Triangle ring on each pole
		leftDown = getIndex(latitudeDensity - 2, 0);
		rightUp = getIndex(0, 1) + 2;
		for (unsigned int lon = 0; lon < longitudeDensity - 1; ++lon, rightUp += 4)
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
		indices.push_back(static_cast<unsigned int>(vertices.Size() - 1));
		indices.push_back(rightUp - 5);
		indices.push_back(2);
		IndexedTriangleList list =
		{
			std::move(vertices), std::move(indices),
			std::string(typeid(Primitive::Sphere).name()) + "UVNla" + std::to_string(latitudeDensity) + "lo" + std::to_string(longitudeDensity)
		};
		list.SetNormals();
		return std::move(list);
	}
	
	IndexedTriangleList Sphere::MakeSolidIco(unsigned int density, const std::vector<VertexAttribute>&& attributes)
	{
		constexpr float bigAngle = M_PI / 10.0f;
		constexpr float smallAngle = M_PI * 0.3f;
		const float baseAngle = atanf(0.5f);

		const float centerZ = cosf(baseAngle);
		const float bigX = centerZ * cosf(bigAngle);
		const float bigZ = centerZ * sinf(bigAngle);
		const float smallX = centerZ * cosf(smallAngle);
		const float smallZ = centerZ * sinf(smallAngle);
		const float level = sinf(baseAngle);

		std::shared_ptr<Data::VertexLayout> layout = std::make_shared<Data::VertexLayout>();
		for (const auto& attrib : attributes)
			layout->Append(attrib);
		Data::VertexBufferData vertices(layout, 12);

		vertices[0].SetByIndex(0, std::move(DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f)));
		vertices[1].SetByIndex(0, std::move(DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f)));
		vertices[2].SetByIndex(0, std::move(DirectX::XMFLOAT3(0.0f, level, -centerZ)));
		vertices[3].SetByIndex(0, std::move(DirectX::XMFLOAT3(-bigX, level, -bigZ)));
		vertices[4].SetByIndex(0, std::move(DirectX::XMFLOAT3(-smallX, level, smallZ)));
		vertices[5].SetByIndex(0, std::move(DirectX::XMFLOAT3(smallX, level, smallZ)));
		vertices[6].SetByIndex(0, std::move(DirectX::XMFLOAT3(bigX, level, -bigZ)));
		vertices[7].SetByIndex(0, std::move(DirectX::XMFLOAT3(0.0f, -level, centerZ)));
		vertices[8].SetByIndex(0, std::move(DirectX::XMFLOAT3(bigX, -level, bigZ)));
		vertices[9].SetByIndex(0, std::move(DirectX::XMFLOAT3(smallX, -level, -smallZ)));
		vertices[10].SetByIndex(0, std::move(DirectX::XMFLOAT3(-smallX, -level, -smallZ)));
		vertices[11].SetByIndex(0, std::move(DirectX::XMFLOAT3(-bigX, -level, bigZ)));

		std::vector<unsigned int> indices
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

		typedef std::pair<unsigned int, unsigned int> Key;
		auto comparator = [](const Key& x, const Key& y) constexpr -> bool
		{
			return (x.first == y.first && x.second == y.second) || (x.first == y.second && x.second == y.first);
		};
		for (unsigned int i = 0; i < density; ++i)
		{
			std::unordered_map<Key, unsigned int, boost::hash<Key>, decltype(comparator)> lookup(0, boost::hash<Key>{}, comparator);
			std::vector<unsigned int> tmpIndices;
			for (unsigned int j = 0; j < indices.size(); j += 3)
			{
				auto i1 = lookup.find({ indices.at(j), indices.at(j + 1) });
				if (i1 == lookup.end())
				{
					i1 = lookup.emplace(std::move(std::make_pair(indices.at(j), indices.at(j + 1))), static_cast<unsigned int>(vertices.Size())).first;
					vertices.EmplaceBack(addNormal(vertices[indices.at(j)].Get<VertexAttribute::Position3D>(),
						vertices[indices.at(j + 1)].Get<VertexAttribute::Position3D>()));
				}
				auto i2 = lookup.find({ indices.at(j + 1), indices.at(j + 2) });
				if (i2 == lookup.end())
				{
					i2 = lookup.emplace(std::move(std::make_pair(indices.at(j + 1), indices.at(j + 2))), static_cast<unsigned int>(vertices.Size())).first;
					vertices.EmplaceBack(addNormal(vertices[indices.at(j + 1)].Get<VertexAttribute::Position3D>(),
						vertices[indices.at(j + 2)].Get<VertexAttribute::Position3D>()));
				}
				auto i3 = lookup.find({ indices.at(j + 2), indices.at(j) });
				if (i3 == lookup.end())
				{
					i3 = lookup.emplace(std::move(std::make_pair(indices.at(j + 2), indices.at(j))), static_cast<unsigned int>(vertices.Size())).first;
					vertices.EmplaceBack(addNormal(vertices[indices.at(j + 2)].Get<VertexAttribute::Position3D>(),
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
		return
		{
			std::move(vertices), std::move(indices),
			std::string(typeid(Primitive::Sphere).name()) + "ICOS" + std::to_string(density)
		};
	}
	
	IndexedTriangleList Sphere::MakeIco(unsigned int density, const std::vector<VertexAttribute>&& attributes)
	{
		constexpr float bigAngle = M_PI / 10.0f;
		constexpr float smallAngle = M_PI * 0.3f;
		const float baseAngle = atanf(0.5f);

		const float centerZ = cosf(baseAngle);
		const float bigX = centerZ * cosf(bigAngle);
		const float bigZ = centerZ * sinf(bigAngle);
		const float smallX = centerZ * cosf(smallAngle);
		const float smallZ = centerZ * sinf(smallAngle);
		const float level = sinf(baseAngle);

		std::shared_ptr<Data::VertexLayout> layout = std::make_shared<Data::VertexLayout>();
		layout->Append(VertexAttribute::Normal);
		for (const auto& attrib : attributes)
			layout->Append(attrib);
		Data::VertexBufferData vertices(layout, 60);
		// 0
		vertices[0].SetByIndex(0, std::move(DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f)));
		vertices[1].SetByIndex(0, std::move(DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f)));
		vertices[2].SetByIndex(0, std::move(DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f)));
		vertices[3].SetByIndex(0, std::move(DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f)));
		vertices[4].SetByIndex(0, std::move(DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f)));
		// 1
		vertices[5].SetByIndex(0, std::move(DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f)));
		vertices[6].SetByIndex(0, std::move(DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f)));
		vertices[7].SetByIndex(0, std::move(DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f)));
		vertices[8].SetByIndex(0, std::move(DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f)));
		vertices[9].SetByIndex(0, std::move(DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f)));
		// 2
		vertices[10].SetByIndex(0, std::move(DirectX::XMFLOAT3(0.0f, level, -centerZ)));
		vertices[11].SetByIndex(0, std::move(DirectX::XMFLOAT3(0.0f, level, -centerZ)));
		vertices[12].SetByIndex(0, std::move(DirectX::XMFLOAT3(0.0f, level, -centerZ)));
		vertices[13].SetByIndex(0, std::move(DirectX::XMFLOAT3(0.0f, level, -centerZ)));
		vertices[14].SetByIndex(0, std::move(DirectX::XMFLOAT3(0.0f, level, -centerZ)));
		// 3
		vertices[15].SetByIndex(0, std::move(DirectX::XMFLOAT3(-bigX, level, -bigZ)));
		vertices[16].SetByIndex(0, std::move(DirectX::XMFLOAT3(-bigX, level, -bigZ)));
		vertices[17].SetByIndex(0, std::move(DirectX::XMFLOAT3(-bigX, level, -bigZ)));
		vertices[18].SetByIndex(0, std::move(DirectX::XMFLOAT3(-bigX, level, -bigZ)));
		vertices[19].SetByIndex(0, std::move(DirectX::XMFLOAT3(-bigX, level, -bigZ)));
		// 4
		vertices[20].SetByIndex(0, std::move(DirectX::XMFLOAT3(-smallX, level, smallZ)));
		vertices[21].SetByIndex(0, std::move(DirectX::XMFLOAT3(-smallX, level, smallZ)));
		vertices[22].SetByIndex(0, std::move(DirectX::XMFLOAT3(-smallX, level, smallZ)));
		vertices[23].SetByIndex(0, std::move(DirectX::XMFLOAT3(-smallX, level, smallZ)));
		vertices[24].SetByIndex(0, std::move(DirectX::XMFLOAT3(-smallX, level, smallZ)));
		// 5
		vertices[25].SetByIndex(0, std::move(DirectX::XMFLOAT3(smallX, level, smallZ)));
		vertices[26].SetByIndex(0, std::move(DirectX::XMFLOAT3(smallX, level, smallZ)));
		vertices[27].SetByIndex(0, std::move(DirectX::XMFLOAT3(smallX, level, smallZ)));
		vertices[28].SetByIndex(0, std::move(DirectX::XMFLOAT3(smallX, level, smallZ)));
		vertices[29].SetByIndex(0, std::move(DirectX::XMFLOAT3(smallX, level, smallZ)));
		// 6
		vertices[30].SetByIndex(0, std::move(DirectX::XMFLOAT3(bigX, level, -bigZ)));
		vertices[31].SetByIndex(0, std::move(DirectX::XMFLOAT3(bigX, level, -bigZ)));
		vertices[32].SetByIndex(0, std::move(DirectX::XMFLOAT3(bigX, level, -bigZ)));
		vertices[33].SetByIndex(0, std::move(DirectX::XMFLOAT3(bigX, level, -bigZ)));
		vertices[34].SetByIndex(0, std::move(DirectX::XMFLOAT3(bigX, level, -bigZ)));
		// 7
		vertices[35].SetByIndex(0, std::move(DirectX::XMFLOAT3(0.0f, -level, centerZ)));
		vertices[36].SetByIndex(0, std::move(DirectX::XMFLOAT3(0.0f, -level, centerZ)));
		vertices[37].SetByIndex(0, std::move(DirectX::XMFLOAT3(0.0f, -level, centerZ)));
		vertices[38].SetByIndex(0, std::move(DirectX::XMFLOAT3(0.0f, -level, centerZ)));
		vertices[39].SetByIndex(0, std::move(DirectX::XMFLOAT3(0.0f, -level, centerZ)));
		// 8
		vertices[40].SetByIndex(0, std::move(DirectX::XMFLOAT3(bigX, -level, bigZ)));
		vertices[41].SetByIndex(0, std::move(DirectX::XMFLOAT3(bigX, -level, bigZ)));
		vertices[42].SetByIndex(0, std::move(DirectX::XMFLOAT3(bigX, -level, bigZ)));
		vertices[43].SetByIndex(0, std::move(DirectX::XMFLOAT3(bigX, -level, bigZ)));
		vertices[44].SetByIndex(0, std::move(DirectX::XMFLOAT3(bigX, -level, bigZ)));
		// 9
		vertices[45].SetByIndex(0, std::move(DirectX::XMFLOAT3(smallX, -level, -smallZ)));
		vertices[46].SetByIndex(0, std::move(DirectX::XMFLOAT3(smallX, -level, -smallZ)));
		vertices[47].SetByIndex(0, std::move(DirectX::XMFLOAT3(smallX, -level, -smallZ)));
		vertices[48].SetByIndex(0, std::move(DirectX::XMFLOAT3(smallX, -level, -smallZ)));
		vertices[49].SetByIndex(0, std::move(DirectX::XMFLOAT3(smallX, -level, -smallZ)));
		// 10
		vertices[50].SetByIndex(0, std::move(DirectX::XMFLOAT3(-smallX, -level, -smallZ)));
		vertices[51].SetByIndex(0, std::move(DirectX::XMFLOAT3(-smallX, -level, -smallZ)));
		vertices[52].SetByIndex(0, std::move(DirectX::XMFLOAT3(-smallX, -level, -smallZ)));
		vertices[53].SetByIndex(0, std::move(DirectX::XMFLOAT3(-smallX, -level, -smallZ)));
		vertices[54].SetByIndex(0, std::move(DirectX::XMFLOAT3(-smallX, -level, -smallZ)));
		// 11
		vertices[55].SetByIndex(0, std::move(DirectX::XMFLOAT3(-bigX, -level, bigZ)));
		vertices[56].SetByIndex(0, std::move(DirectX::XMFLOAT3(-bigX, -level, bigZ)));
		vertices[57].SetByIndex(0, std::move(DirectX::XMFLOAT3(-bigX, -level, bigZ)));
		vertices[58].SetByIndex(0, std::move(DirectX::XMFLOAT3(-bigX, -level, bigZ)));
		vertices[59].SetByIndex(0, std::move(DirectX::XMFLOAT3(-bigX, -level, bigZ)));

		std::vector<unsigned int> indices
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

		for (unsigned int i = 0; i < density; ++i)
		{
			std::vector<unsigned int> tmpIndices;
			for (unsigned int j = 0; j < indices.size(); j += 3)
			{
				Data::Vertex v0 = vertices[indices.at(j)];
				Data::Vertex v1 = vertices[indices.at(j + 1)];
				Data::Vertex v2 = vertices[indices.at(j + 2)];
				DirectX::XMFLOAT3 left = addNormal(v0.Get<VertexAttribute::Position3D>(), v1.Get<VertexAttribute::Position3D>());
				DirectX::XMFLOAT3 right = addNormal(v1.Get<VertexAttribute::Position3D>(), v2.Get<VertexAttribute::Position3D>());
				DirectX::XMFLOAT3 down = addNormal(v2.Get<VertexAttribute::Position3D>(), v0.Get<VertexAttribute::Position3D>());
				const unsigned int id = static_cast<unsigned int>(vertices.Size());
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
		IndexedTriangleList list =
		{
			std::move(vertices), std::move(indices),
			std::string(typeid(Primitive::Sphere).name()) + "ICON" + std::to_string(density)
		};
		list.SetNormals();
		return std::move(list);
	}
}