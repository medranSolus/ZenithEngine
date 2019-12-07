#pragma once
#include "IndexedTriangleList.h"
#include <cmath>
#include <unordered_map>
#include <boost/functional/hash/hash.hpp>

namespace GFX::Primitive
{
	class Sphere
	{
	public:
		//latitudeDensity: N-S, longitudeDensity: W-E
		template<typename V>
		static IndexedTriangleList<V> MakeSolidUV(unsigned int latitudeDensity, unsigned int longitudeDensity)
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

			std::vector<V> vertices;
			// Sphere vertices without poles
			for (unsigned int lat = 1; lat < latitudeDensity; ++lat)
			{
				const auto latBase = DirectX::XMVector3Transform(base, DirectX::XMMatrixRotationX(latitudeAngle * lat));
				for (unsigned int lon = 0; lon < longitudeDensity; ++lon)
				{
					vertices.emplace_back();
					DirectX::XMStoreFloat3(&vertices.back().pos, std::move(DirectX::XMVector3Transform(latBase, DirectX::XMMatrixRotationY(longitudeAngle * lon))));
				}
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
			const unsigned int pole = static_cast<unsigned int>(vertices.size());
			// South
			vertices.emplace_back();
			DirectX::XMStoreFloat3(&vertices.back().pos, std::move(DirectX::XMVectorNegate(base)));
			// Norht
			vertices.emplace_back();
			DirectX::XMStoreFloat3(&vertices.back().pos, std::move(base));

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
			return { std::move(vertices), std::move(indices) };
		}

		//latitudeDensity: N-S, longitudeDensity: W-E
		template<typename V>
		static IndexedTriangleList<V> MakeUV(unsigned int latitudeDensity, unsigned int longitudeDensity)
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

			std::vector<V> vertices;
			// Sphere vertices without poles
			for (unsigned int lat = 1; lat < latitudeDensity; ++lat)
			{
				const auto latBase = DirectX::XMVector3Transform(base, DirectX::XMMatrixRotationX(latitudeAngle * lat));
				for (unsigned int lon = 0; lon < longitudeDensity; ++lon)
				{
					const DirectX::XMVECTOR vec = DirectX::XMVector3Transform(latBase, DirectX::XMMatrixRotationY(longitudeAngle * lon));
					for (unsigned char i = 0; i < 4; ++i)
					{
						vertices.emplace_back();
						DirectX::XMStoreFloat3(&vertices.back().pos, vec);
					}
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
			const unsigned int south = static_cast<unsigned int>(vertices.size());
			for (unsigned char i = 0; i < longitudeDensity; ++i)
			{
				vertices.emplace_back();
				DirectX::XMStoreFloat3(&vertices.back().pos, std::move(DirectX::XMVectorNegate(base)));
			}
			const unsigned int north = static_cast<unsigned int>(vertices.size());
			for (unsigned char i = 0; i < longitudeDensity; ++i)
			{
				vertices.emplace_back();
				DirectX::XMStoreFloat3(&vertices.back().pos, std::move(base));
			}

			// Triangle ring on each pole
			leftDown = getIndex(latitudeDensity - 1, 0);
			rightUp = getIndex(0, 1) + 2;
			for (unsigned int lon = 0; lon < longitudeDensity - 1; ++lon, ++leftDown, rightUp += 4)
			{
				// North
				indices.push_back(leftDown);
				indices.push_back(north + lon);
				leftDown += 7;
				indices.push_back(leftDown);
				// South
				indices.push_back(south + lon);
				indices.push_back(rightUp - 5);
				indices.push_back(rightUp);
			}
			// Last triangle to connect ring
			// North
			indices.push_back(leftDown);
			indices.push_back(vertices.size() - 1);
			indices.push_back(getIndex(latitudeDensity - 1, 0) + 3);
			// South
			indices.push_back(north - 1);
			indices.push_back(rightUp - 5);
			indices.push_back(2);
			return { std::move(vertices), std::move(indices) };
		}

		template<typename V>
		static IndexedTriangleList<V> MakeSolidIco(unsigned int density)
		{
			const float root = sqrtf(5.0f);
			const float bigX = sqrtf((5.0f + root) / 8.0f);
			const float bigZ = (root - 1) / 4.0f;;
			const float smallX = sqrtf((5.0f - root) / 8.0f);
			const float smallZ = (root + 1) / 4.0f;
			const float level = smallX * sqrtf(3.0f) / 2.0f;
			const float poleY = sqrtf(4.0f * level * level - smallX * smallX * (1.0f + 2.0f * root / 5.0f)) + level;
			std::vector<V> vertices
			{
				{{ 0.0f, poleY, 0.0f }},        // 0
				{{ 0.0f, -poleY, 0.0f }},       // 1

				{{ 0.0f, level, -1.0f }},       // 2
				{{ -bigX, level, -bigZ }},      // 3
				{{ -smallX, level, smallZ }},   // 4
				{{ smallX, level, smallZ }},    // 5
				{{ bigX, level, -bigZ }},       // 6

				{{ 0.0f, -level, 1.0f }},       // 7
				{{ bigX, -level, bigZ }},       // 8
				{{ smallX, -level, -smallZ }},  // 9
				{{ -smallX, -level, -smallZ }}, // 10
				{{ -bigX, -level, bigZ }}       // 11
			};
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
			auto comparator = [](const Key & x, const Key & y) constexpr -> bool
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
						i1 = lookup.emplace(std::move(std::make_pair(indices.at(j), indices.at(j + 1))), static_cast<unsigned int>(vertices.size())).first;
						V middle = vertices.at(indices.at(j)) + vertices.at(indices.at(j + 1));
						vertices.emplace_back(middle *= (poleY / middle()));
					}
					auto i2 = lookup.find({ indices.at(j + 1), indices.at(j + 2) });
					if (i2 == lookup.end())
					{
						i2 = lookup.emplace(std::move(std::make_pair(indices.at(j + 1), indices.at(j + 2))), static_cast<unsigned int>(vertices.size())).first;
						V middle = vertices.at(indices.at(j + 1)) + vertices.at(indices.at(j + 2));
						vertices.emplace_back(middle *= (poleY / middle()));
					}
					auto i3 = lookup.find({ indices.at(j + 2), indices.at(j) });
					if (i3 == lookup.end())
					{
						i3 = lookup.emplace(std::move(std::make_pair(indices.at(j + 2), indices.at(j))), static_cast<unsigned int>(vertices.size())).first;
						V middle = vertices.at(indices.at(j + 2)) + vertices.at(indices.at(j));
						vertices.emplace_back(middle *= (poleY / middle()));
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
	
		template<typename V>
		static IndexedTriangleList<V> MakeIco(unsigned int density)
		{
			const float root = sqrtf(5.0f);
			const float bigX = sqrtf((5.0f + root) / 8.0f);
			const float bigZ = (root - 1) / 4.0f;;
			const float smallX = sqrtf((5.0f - root) / 8.0f);
			const float smallZ = (root + 1) / 4.0f;
			const float level = smallX * sqrtf(3.0f) / 2.0f;
			const float poleY = sqrtf(4.0f * level * level - smallX * smallX * (1.0f + 2.0f * root / 5.0f)) + level;
			std::vector<V> vertices
			{
				// 0
				{{ 0.0f, poleY, 0.0f }}, // 0
				{{ 0.0f, poleY, 0.0f }}, // 1
				{{ 0.0f, poleY, 0.0f }}, // 2
				{{ 0.0f, poleY, 0.0f }}, // 3
				{{ 0.0f, poleY, 0.0f }}, // 4
				// 1
				{{ 0.0f, -poleY, 0.0f }}, // 5
				{{ 0.0f, -poleY, 0.0f }}, // 6
				{{ 0.0f, -poleY, 0.0f }}, // 7
				{{ 0.0f, -poleY, 0.0f }}, // 8
				{{ 0.0f, -poleY, 0.0f }}, // 9
				// 2
				{{ 0.0f, level, -1.0f }}, // 10
				{{ 0.0f, level, -1.0f }}, // 11
				{{ 0.0f, level, -1.0f }}, // 12
				{{ 0.0f, level, -1.0f }}, // 13
				{{ 0.0f, level, -1.0f }}, // 14
				// 3
				{{ -bigX, level, -bigZ }}, // 15
				{{ -bigX, level, -bigZ }}, // 16
				{{ -bigX, level, -bigZ }}, // 17
				{{ -bigX, level, -bigZ }}, // 18
				{{ -bigX, level, -bigZ }}, // 19
				// 4
				{{ -smallX, level, smallZ }}, // 20
				{{ -smallX, level, smallZ }}, // 21
				{{ -smallX, level, smallZ }}, // 22
				{{ -smallX, level, smallZ }}, // 23
				{{ -smallX, level, smallZ }}, // 24
				// 5
				{{ smallX, level, smallZ }}, // 25
				{{ smallX, level, smallZ }}, // 26
				{{ smallX, level, smallZ }}, // 27
				{{ smallX, level, smallZ }}, // 28
				{{ smallX, level, smallZ }}, // 29
				// 6
				{{ bigX, level, -bigZ }}, // 30
				{{ bigX, level, -bigZ }}, // 31
				{{ bigX, level, -bigZ }}, // 32
				{{ bigX, level, -bigZ }}, // 33
				{{ bigX, level, -bigZ }}, // 34
				// 7
				{{ 0.0f, -level, 1.0f }}, // 35
				{{ 0.0f, -level, 1.0f }}, // 36
				{{ 0.0f, -level, 1.0f }}, // 37
				{{ 0.0f, -level, 1.0f }}, // 38
				{{ 0.0f, -level, 1.0f }}, // 39
				// 8
				{{ bigX, -level, bigZ }}, // 40
				{{ bigX, -level, bigZ }}, // 41
				{{ bigX, -level, bigZ }}, // 42
				{{ bigX, -level, bigZ }}, // 43
				{{ bigX, -level, bigZ }}, // 44
				// 9
				{{ smallX, -level, -smallZ }}, // 45
				{{ smallX, -level, -smallZ }}, // 46
				{{ smallX, -level, -smallZ }}, // 47
				{{ smallX, -level, -smallZ }}, // 48
				{{ smallX, -level, -smallZ }}, // 49
				// 10
				{{ -smallX, -level, -smallZ }}, // 50
				{{ -smallX, -level, -smallZ }}, // 51
				{{ -smallX, -level, -smallZ }}, // 52
				{{ -smallX, -level, -smallZ }}, // 53
				{{ -smallX, -level, -smallZ }}, // 54
				// 11
				{{ -bigX, -level, bigZ }}, // 55
				{{ -bigX, -level, bigZ }}, // 56
				{{ -bigX, -level, bigZ }}, // 57
				{{ -bigX, -level, bigZ }}, // 58
				{{ -bigX, -level, bigZ }}, // 59
			};
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
				40, 5, 36,
				45, 6, 41,
				50, 7, 46,
				55, 8, 51,
				35, 9, 56
			};

			for (unsigned int i = 0; i < density; ++i)
			{
				std::vector<unsigned int> tmpIndices;
				for (unsigned int j = 0; j < indices.size(); j += 3)
				{
					V & v0 = vertices.at(indices.at(j));
					V & v1 = vertices.at(indices.at(j + 1));
					V & v2 = vertices.at(indices.at(j + 2));
					V left = v0 + v1;
					V right = v1 + v2;
					V down = v2 + v0;
					left *= poleY / left();
					right *= poleY / right();
					down *= poleY / down();
					const unsigned int id = vertices.size();
					vertices.emplace_back(left);  // 0
					vertices.emplace_back(left);  // 1
					vertices.emplace_back(left);  // 2
					vertices.emplace_back(right); // 3
					vertices.emplace_back(right); // 4
					vertices.emplace_back(right); // 5
					vertices.emplace_back(down);  // 6
					vertices.emplace_back(down);  // 7
					vertices.emplace_back(down);  // 8

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
			IndexedTriangleList<V> list = { std::move(vertices), std::move(indices) };
			list.SetNormals();
			return std::move(list);
		}
};
}
