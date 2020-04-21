#pragma once
#include "IndexedTriangleList.h"

namespace GFX::Primitive
{
	class Sphere
	{
	public:
		Sphere() = delete;

		//latitudeDensity: N-S, longitudeDensity: W-E
		static IndexedTriangleList MakeSolidUV(unsigned int latitudeDensity, unsigned int longitudeDensity, const std::vector<VertexAttribute>&& attributes = {});
		//latitudeDensity: N-S, longitudeDensity: W-E
		static IndexedTriangleList MakeUV(unsigned int latitudeDensity, unsigned int longitudeDensity, const std::vector<VertexAttribute>&& attributes = {});

		static IndexedTriangleList MakeSolidIco(unsigned int density, const std::vector<VertexAttribute>&& attributes = {});
		static IndexedTriangleList MakeIco(unsigned int density, const std::vector<VertexAttribute>&& attributes = {});
	};
}